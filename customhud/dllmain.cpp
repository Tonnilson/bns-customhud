// dllmain.cpp : Defines the entry point for the DLL application.
#include <pe/module.h>
#include <xorstr.hpp>
#include "pluginsdk.h"
#include "searchers.h"
#include <tchar.h>
#include <iostream>
#include <ShlObj.h>
#include <KnownFolders.h>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <wil/stl.h>
#include <wil/win32_helpers.h>
#include "detours.h"

#include "UIElement.h"
#include "functions.h"

static bool Hide_UI_Panels = false;
bool SHOW_NAMEPLATES = true;
float ScreenH = 0.0f;
float ScreenW = 0.0f;
int* ZoneID;
static std::filesystem::path FilterPath;

const std::filesystem::path& documents_path()
{
	static std::once_flag once_flag;
	static std::filesystem::path path;

	std::call_once(once_flag, [](std::filesystem::path& path) {
		wil::unique_cotaskmem_string result;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &result)))
			path = result.get();
		}, path);
	return path;
}

// Common macros
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & (1 << 16)))
#define IN_RANGE(low, high, x) ((low <= x && x <= high))
#define IS_NUMERIC(string) (!string.empty() && std::find_if(string.begin(), string.end(), [](unsigned char c) { return !std::isdigit(c); }) == string.end())

// Setup our vectors
static std::vector<UIPanel*>UI_PANELS;
static std::vector<int>LayerIds = {
	//24 // Premium Buff, Clan Bonus and premium food buffs ( This is not reliable but most of the time its this... )
};

static std::vector<SCompoundWidget*> LayersTable;
bool UiStateInit = false;
static int DepthOfFieldIndex = 0;

static std::vector<std::wstring> PANEL_NAMES = {
	L"PlayerExpBarPanel",
	L"PlayerStatusPanel",
	L"ChattingPanel",
	L"QuestQuickSlotPanel",
	L"SystemMenuPanel",
	L"NotificationMenuPanel",
	L"ItemBarPanel",
	L"SprintPanel"
};

static void loadFilter() {
	std::wstring line;
	std::wifstream stream(FilterPath.c_str());

	// Clear out our list
	if (!PANEL_NAMES.empty())
		PANEL_NAMES.clear();
	if (!LayerIds.empty())
		LayerIds.clear();

	while (std::getline(stream, line)) {
		if (!line.empty() && line[0] != '#') {
			if (IS_NUMERIC(line))
				LayerIds.push_back(std::stoi(line));
			else
				PANEL_NAMES.push_back(line);
		}
	}

	stream.close(); // Something I forgot to do with fpsbooster was close the file stream
	return;
}

// I should replace this with a directinput hook to read the key states of the game
// But I have other projects already hooking it and I don't want to conflict with potential future plugins that will make better use

bool ToggleUiPressed = false;
bool ToggleNamePlates = false;
bool ReloadList = false;

bool __fastcall hkBInputKey(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent) {
	// Don't bother if the alt key isn't even pressed.
	// This will also not fire if a key is pressed before Alt which to be honest I am fine with
	if (InputKeyEvent->bAltPressed) {
		switch (InputKeyEvent->_vKey)  {
			case VK_INSERT:
				if (!ReloadList && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					ReloadList = true;
					if (std::filesystem::exists(FilterPath))
						loadFilter();
				}
				else if (ReloadList && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					ReloadList = false;
				break;
			case 0x58: // X Key
				if (!ToggleUiPressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					ToggleUiPressed = true;
					if (UiStateGamePtr != NULL) {
						Hide_UI_Panels = !Hide_UI_Panels;
						for (auto panel : UI_PANELS) {
							if (panel == NULL) continue; // ptr null skip
							if (panel->panelName == NULL) continue; // name is null but ptr could be fine, regardless skip cause it's not what we want

							const wchar_t* name = panel->panelName;
							auto result = std::any_of(PANEL_NAMES.begin(), PANEL_NAMES.end(), [name](std::wstring elem) {
								return wcscmp(elem.c_str(), name) == 0;
							});

							if (!result) continue;
							// In celestial basin skip over this one
							if (wcscmp(name, L"QuestQuickSlotPanel") == 0 && ZoneID && IN_RANGE(6400, 6410, *ZoneID)) continue;
							//bool isPanelVisible = (unsigned __int8)oUIPanelIsVisible(panel, false) == 1;
							// Toggle can work but it's weird
							oUIPanelToggle(panel, true);
							// Can also use UiStateGame::ShowPanel to show or hide a panel but need to know the ptr for the uistategame obj and pass it
							//oUiStateGameShowPanel(UiStateGamePtr, panel, !Hide_UI_Panels, true, true);
						}

						// Rehides the Depth of field filter if it's not hidden
						if (LayersTable[DepthOfFieldIndex]->IsVisible != 0)
							LayersTable[DepthOfFieldIndex]->IsVisible = 0;

						// I am not 100% sure exactly what these are, they're not what I initially thought they were but they are called via Ctrl + X
						// I have observed no negatives to hiding them.
						if (*BNSClientInstance) {
							if (!BNSInstance)
								BNSInstance = *(BInstance**)BNSClientInstance;

							// Make sure the pointer is valid and the functions are assigned
							if (*BNSInstance->PresentationWorld && oSetEnableBalloon && oSetEnableIndicator) {
								oSetEnableIndicator(BNSInstance->PresentationWorld, !Hide_UI_Panels); // Pretty sure this is quest icon stuff above npc's heads
								oSetEnableBalloon(BNSInstance->PresentationWorld, !Hide_UI_Panels); // And this should be the text bubble above their heads
							}
						}

						// I don't recommend this
						if (!LayerIds.empty()) {
							for (auto layer : LayerIds) {
								if (layer > LayersTable.size() - 1) continue; // index is out of range so ignore
								LayersTable[layer]->IsVisible = (LayersTable[layer]->IsVisible == 0) ? 17 : 0; // 0 = don't show | val > 0 < 17 = show but no interaction | val >= 17 = show with interaction enabled
								// We also set the alpha level to 0 if needed
								// Why? Widgets that are always being updated won't let you change the IsVisible or Opacity property so instead we go for Alpha channel of the ColorDrawArgs
								// Could probably do this better and call internal functions to make a widget visible or not but too lazy to go through all of that.
								LayersTable[layer]->AlphaLevel = (LayersTable[layer]->AlphaLevel == 0.0f) ? 1.0f : 0.0f;
							}
						}
					}
				}
				else if (ToggleUiPressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					ToggleUiPressed = false;

				break;
			case 0x42: // B Key
				if (!oSetEnableNamePlate) break;
				if (!ToggleNamePlates && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					ToggleNamePlates = true;
					if (*BNSClientInstance) {
						if (!BNSInstance)
							BNSInstance = *(BInstance**)BNSClientInstance;

						// Make sure the pointer is valid
						if (*BNSInstance->PresentationWorld) {
							oSetEnableNamePlate(BNSInstance->PresentationWorld, !SHOW_NAMEPLATES);
							SHOW_NAMEPLATES = !SHOW_NAMEPLATES;
						}
					}
				}
				else if (ToggleNamePlates && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					ToggleNamePlates = false;

				break;
		}
	}
	return oBInputKey(thisptr, InputKeyEvent);
}

// Called when loading new world, hooking this to get UiGameState object pointer for later use and other things
bool _fastcall hkUiStateGame(__int64 thisptr) {
	if (thisptr) {
		UiStateGamePtr = thisptr;
		// Reset some internal stuff on each new load
		Hide_UI_Panels = false;
		SHOW_NAMEPLATES = true;
		// This is for SCompoundWidget::OnPaint
		UiStateInit = true;
		TargetedLayer = NULL;
		// We're going to empty our list of pointers cause we loaded into a new world
		if (!UI_PANELS.empty())
			UI_PANELS.clear();

		if (!LayersTable.empty())
			LayersTable.clear();
		//std::cout << "UiStateGame::UiStateGame() executed" << std::endl;
	}
	return oUiStateGame(thisptr);
}

// Hooking this to get list of pointers that will be used
// Could go loop through the vtable for UiStateGame but that requires documenting offsets and other things
// Much easier to do this so not reliant on offsets which could change at any time a new panel is added to the game
bool __fastcall hkUIPanelSetName(UIPanel* uiPanel, const wchar_t* name) {
	if (uiPanel) {
		//printf("Panel Created: %S | %p\n", name, uiPanel);
		UI_PANELS.push_back(uiPanel);
	}

	return oUIPanelSetName(uiPanel, name);
}

// Used this for debugging purposes
/*
bool __fastcall hkUiStateGameShowPanel(__int64 UiStateGame, UIPanel* panel, bool show, bool playEvent, bool processPanelShowEvent) {
	if (panel != NULL)
		if(panel->panelName == NULL)
			printf("Panel ID: %d | Show: %d | playEvent: %d | processPanelShowEvent: %d\n", panel->panelId, show, playEvent, processPanelShowEvent);
		else
			printf("Panel: %S | Show: %d | playEvent: %d | processPanelShowEvent: %d\n", panel->panelName, show, playEvent, processPanelShowEvent);

	return oUiStateGameShowPanel(UiStateGame, panel, show, playEvent, processPanelShowEvent);
}
*/

// Hooking this to get widget layers, normal people would get the vtable and iterate over that but it's kind of big and I don't really want to document the offsets..
// Instead I make my own table of pointers cause jank shit fuck yeah. The potential for offsets to change on each load are quite high.. but whatever works most of the time.
bool __fastcall hkSCompoundWidgetOnPaint(SCompoundWidget* thisptr, __int64 Args, __int64 AllotedGeometry, __int64 MyCullingRect, __int64 OutDrawElements, int LayerId, __int64 InWidgetStyle, bool bParentEnabled) {
	if (UiStateInit) {
		if (UiStateGamePtr != NULL) {
			// We stop collecting after so much because it's not needed to know
			if (LayersTable.size() >= 82) {
				UiStateInit = false;
				// Sort our jank list by layerId
				std::sort(LayersTable.begin(), LayersTable.end(), [](SCompoundWidget* v1, SCompoundWidget* v2) { return v1->LayerID < v2->LayerID; });

				// Fill in some missing info
				if (ScreenH == 0.0f) {
					ScreenH = LayersTable[6]->HSize;
					ScreenW = LayersTable[6]->VSize;
				}

				// The indexes obviously change depending on what was set as the initial layers but the range is relatively the same
				// So loop through index 56-63 and make sure the parent is not a master layer
				// Check the width (max horizontal) to make sure it matches render resolution
				for (int i = 56; i < 63; i++) {
					if (LayersTable[i]->Parent->LayerID > 19 && LayersTable[i]->HSize == ScreenH && LayersTable[i]->VSize == ScreenW) {
						DepthOfFieldIndex = i;
						break;
					}
				}
				TargetedLayer = reinterpret_cast<SCompoundWidget*>(LayersTable[DepthOfFieldIndex]);
				TargetedLayer->IsVisible = 0;
			}

			if (std::none_of(LayersTable.begin(), LayersTable.end(), [thisptr](SCompoundWidget* x) { return x == thisptr; }))
				LayersTable.push_back(thisptr);
		}
	}

	return oSCompoundWidgetOnPaint(thisptr, Args, AllotedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void __cdecl oep_notify([[maybe_unused]] const Version client_version)
{
	if (const auto module = pe::get_module()) {
		DetourTransactionBegin();
		DetourUpdateThread(NtCurrentThread());

		uintptr_t handle = module->handle();
		const auto sections2 = module->segments();
		const auto& s2 = std::find_if(sections2.begin(), sections2.end(), [](const IMAGE_SECTION_HEADER& x) {
			return x.Characteristics & IMAGE_SCN_CNT_CODE;
			});
		const auto data = s2->as_bytes();

		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);

		// If filter exists, load custom filter set.
		if (std::filesystem::exists(FilterPath))
			loadFilter();

		auto sBinput = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("0F B6 47 18 48 8D 4C 24 30 89 03")));
		if (sBinput != data.end()) {
			uintptr_t aBinput = (uintptr_t)&sBinput[0] - 0x38;
			oBInputKey = module->rva_to<std::remove_pointer_t<decltype(oBInputKey)>>(aBinput - handle);
			DetourAttach(&(PVOID&)oBInputKey, &hkBInputKey);
		}

		//40 53 48 83 EC 30 48 8B DA 0F 29 74 24 20 48 8D 15 + 0x53 
		// 44 8B 05 B2 66 E1 05
		auto sZonePtr = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("40 53 48 83 EC 30 48 8B DA 0F 29 74 24 20 48 8D 15")));
		if (sZonePtr != data.end()) {
			ZoneID = reinterpret_cast<int*>(GetAddress((uintptr_t)&sZonePtr[0] + 0x53, 3, 7));
			std::cout << "ZoneID | " << *ZoneID << std::endl;
		}

		// Stops invokeGameMessage for combat log
		// Same as with FPSBooster, we stop this function from running as it does the lookup process and everything else before checking if combat log printing was disabled where as it should be done after
		// Unclear whether or not this provides an FPS bonus like it did with the UE3 client but still including it because who really uses the combat log.
		auto sMHandler = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("49 89 43 C0 4D 89 4B B8 4D 8B C8 41 B8 08 00 00 00")));
		if (sMHandler != data.end()) {
			uintptr_t aHandler = (uintptr_t)&sMHandler[0] - 0x4B;
			BYTE ret[] = { 0xC3, 0x90, 0x90 };
			DWORD oldprotect;
			VirtualProtect((LPVOID)aHandler, sizeof(ret), PAGE_EXECUTE_READWRITE, &oldprotect);
			memcpy((LPVOID)aHandler, ret, sizeof(ret));
			VirtualProtect((LPVOID)aHandler, sizeof(ret), oldprotect, &oldprotect);
		}

		auto sSCompoundOnPaint = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("F3 0F 10 4C 24 4C F3 0F 59 8F 34 03 00 00 F3 0F 11 4C 24 4C")));
		if (sSCompoundOnPaint != data.end()) {
			uintptr_t aOnPaint = (uintptr_t)&sSCompoundOnPaint[0] - 0x19C;
			oSCompoundWidgetOnPaint = module->rva_to<std::remove_pointer_t<decltype(oSCompoundWidgetOnPaint)>>(aOnPaint - handle);
			DetourAttach(&(PVOID&)oSCompoundWidgetOnPaint, &hkSCompoundWidgetOnPaint);
		}

		auto sUIPanelSetName = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 5C 24 10 57 48 83 EC 20 48 8B DA 48 8B F9 49 C7 C0 FF FF FF FF 66 0F 1F 84 00 00 00 00 00")));
		uintptr_t aUIPanelSetName = NULL;
		if (sUIPanelSetName != data.end()) {
			//std::cout << "Hooking UiPanelSetName" << std::endl;
			aUIPanelSetName = (uintptr_t)&sUIPanelSetName[0];
			oUIPanelSetName = module->rva_to<std::remove_pointer_t<decltype(oUIPanelSetName)>>(aUIPanelSetName - handle);
			DetourAttach(&(PVOID&)oUIPanelSetName, &hkUIPanelSetName);
		}

		auto sUIPanelToggle = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("84 C0 41 B0 01 48 8B CB 0F 94 C2 48 83 C4 20 5B")));
		if (sUIPanelToggle != data.end()) {
			// These two should be side by side unless they add something between this in the future
			uintptr_t aUIPanelToggle = (uintptr_t)&sUIPanelToggle[0] - 0x29;
			uintptr_t aUIPanelIsVisible = (uintptr_t)&sUIPanelToggle[0] + 0x17;
			oUIPanelToggle = module->rva_to<std::remove_pointer_t<decltype(oUIPanelToggle)>>(aUIPanelToggle - handle);
			oUIPanelIsVisible = module->rva_to<std::remove_pointer_t<decltype(oUIPanelIsVisible)>>(aUIPanelIsVisible - handle);
		}

		auto sUiStateGame = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 4C 24 08 55 56 57 48 8B EC 48 83 EC 40 48 C7 45 F0 FE FF FF FF")));
		if (sUiStateGame != data.end()) {
			//std::cout << "Hooking UiStateGame" << std::endl;
			uintptr_t aUiStateGame = (uintptr_t)&sUiStateGame[0];
			oUiStateGame = module->rva_to<std::remove_pointer_t<decltype(oUiStateGame)>>(aUiStateGame - handle);
			DetourAttach(&(PVOID&)oUiStateGame, &hkUiStateGame);
		}

		auto sShowPanel = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 41 0F B6 F9 41 0F B6 F0 48 8B")));
		if (sShowPanel != data.end()) {
			//std::cout << "Found UiStateGame::showPanel" << std::endl;
			oUiStateGameShowPanel = module->rva_to<std::remove_pointer_t<decltype(oUiStateGameShowPanel)>>((uintptr_t)&sShowPanel[0] - handle);
			//DetourAttach(&(PVOID&)oUiStateGameShowPanel, &hkUiStateGameShowPanel);
		}

		auto sBShowHud = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("45 32 D2 32 DB 48 89 6C 24 38 44 8B CE 85")));
		if (sBShowHud != data.end()) {
			BNSClientInstance = (uintptr_t*)GetAddress((uintptr_t)&sBShowHud[0] + 0x5A, 3, 7);
		}

		// 48 89 5C 24 30 48 89 7C 24 38 88 91 A0 00 00 00 - 0x1C (SetEnableNamePlate)
		// 48 85 D2 0F B6 D3 48 0F 44 CD E8 (End of BUiWorld::ShowHud)
		auto sSetNamePlate = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 85 D2 0F B6 D3 48 0F 44 CD E8")));
		if (sSetNamePlate != data.end()) {
			uintptr_t aSetNamePlate = (uintptr_t)&sSetNamePlate[0] - 0x66;
			uintptr_t aSetBalloon = (uintptr_t)&sSetNamePlate[0] - 0x86;
			uintptr_t aSetIndicator = (uintptr_t)&sSetNamePlate[0] - 0x46;

			// Make sure it is a CALL before trying to get the full address of the call's relative address
			if(*reinterpret_cast<BYTE*>(aSetNamePlate) == 0xE8)
				oSetEnableNamePlate = module->rva_to<std::remove_pointer_t<decltype(oSetEnableNamePlate)>>(GetAddress(aSetNamePlate, 1, 5) - handle);

			if (*reinterpret_cast<BYTE*>(aSetBalloon) == 0xE8)
				oSetEnableBalloon = module->rva_to<std::remove_pointer_t<decltype(oSetEnableBalloon)>>(GetAddress(aSetBalloon, 1, 5) - handle);

			if (*reinterpret_cast<BYTE*>(aSetIndicator) == 0xE8)
				oSetEnableIndicator = module->rva_to<std::remove_pointer_t<decltype(oSetEnableIndicator)>>(GetAddress(aSetIndicator, 1, 5) - handle);
		}

		DetourTransactionCommit();
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (FilterPath.empty())
			FilterPath = documents_path() / xorstr_(L"BnS\\customhud.txt");

		DisableThreadLibraryCalls(hInstance);
	}

	return TRUE;
}

bool __cdecl init([[maybe_unused]] const Version client_version)
{
	NtCurrentPeb()->BeingDebugged = FALSE;
	return true;
}

extern "C" __declspec(dllexport) PluginInfo GPluginInfo = {
  .hide_from_peb = true,
  .erase_pe_header = true,
  .init = init,
  .oep_notify = oep_notify,
  .priority = 1,
  .target_apps = L"BNSR.exe"
};