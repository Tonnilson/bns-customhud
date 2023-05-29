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
#include <regex>

#include "UIEngineInterface.h"
#include "UIElement.h"
#include "functions.h"
#include "World.h"

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 0
typedef NTSTATUS(NTAPI* tLdrRegisterDllNotification)(ULONG, PVOID, PVOID, PVOID);
void NTAPI DllNotification(ULONG notification_reason, const LDR_DLL_NOTIFICATION_DATA* notification_data, PVOID context);
static bool Hide_UI_Panels = false;
bool SHOW_NAMEPLATES = true;
float ScreenH = 0.0f;
float ScreenW = 0.0f;
int* ZoneID;
static std::filesystem::path FilterPath;

bool bDisableText = false;

uintptr_t CombatLogMessageHandler = NULL;
BYTE combatlog_original[] = { 0x00, 0x00, 0x00 };
BYTE combatlog_turnedoff[] = { 0xC3, 0x90, 0x90 };
uintptr_t* UIEngineInterfacePtr = NULL;

void __fastcall AddNotification(const wchar_t* text,
	const wchar_t* particleRef,
	const wchar_t* sound,
	char track,
	bool stopPreviousSound,
	bool headline2,
	bool boss_headline,
	bool chat,
	char category,
	const wchar_t* sound2) {
	auto World = BNSClient_GetWorld();
	if (World && oAddInstantNotification)
		oAddInstantNotification(World, text, particleRef, sound, track, stopPreviousSound, headline2, boss_headline, chat, category, sound2);
}

// This is lazy code to keep the method the same even tho the regions changed methods
PresentationWorld* GetPresentationWorld() {
	if (BNSInstance)
		return BNSInstance->PresentationWorld ? (PresentationWorld*)BNSInstance->PresentationWorld : 0;
	else
		return 0;
}

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
	L"Feedback_CombatSignal_Panel",
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

bool ToggleUiPressed = false;
bool ToggleNamePlates = false;
bool ReloadList = false;
bool toggleFilter = false;
bool CombatLog = false;
bool dText = false;

bool __fastcall hkBInputKey(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent) {
	// Don't bother if the alt key isn't even pressed.
	// This will also not fire if a key is pressed before Alt which to be honest I am fine with
	if (InputKeyEvent->bAltPressed) {
		switch (InputKeyEvent->_vKey)  {
			case VK_INSERT:
				if (!ReloadList && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					ReloadList = true;
					if (std::filesystem::exists(FilterPath)) {
						loadFilter();
						AddNotification(
							L"<font name=\"00008130.UI.Label_Red02_20_out\">Reloading panel list</font>", // Msg
							L"", // Particle Ref
							L"", // Sound
							0, // Track
							false, // Stop Previous Sound
							false, // Headline2
							false, // Boss Headline
							false, // Chat
							0, // Other Category type if none of the above (0 = Scrolling Text headline)
							L"" // Sound 2
						);
					}
				}
				else if (ReloadList && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					ReloadList = false;
				break;
			case 0x58: // X Key
				if (!ToggleUiPressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					ToggleUiPressed = true;

					auto World = BNSClient_GetWorld();
					if (World) {
						auto Instance = (IUIEngineInterface*)oUIEngineInterfaceGetInstance();
						if (Instance) {
							Hide_UI_Panels = !Hide_UI_Panels;
							for (auto panelName : PANEL_NAMES) {
								auto Id = Instance->vfptr->GetId(Instance, panelName.c_str());

								if (wcscmp(panelName.c_str(), L"QuestQuickSlotPanel") == 0 && World && IN_RANGE(6400, 6410, World->_geozoneId)) continue;

								if (Hide_UI_Panels && Id) {
									Instance->vfptr->Hide(Instance, Id, false);
								}
								else if (!Hide_UI_Panels && Id)
									BnsUIEngineInterfaceImpl_Show(Instance, Id, false);
							}

							// I am not 100% sure exactly what these are, they're not what I initially thought they were but they are called via Ctrl + X
							// I have observed no negatives to hiding them.
							auto PresentationWorld = BNSClient_GetPresentationWorld();
							if (PresentationWorld) {
								oSetEnableIndicator(PresentationWorld, !Hide_UI_Panels); // Pretty sure this is quest icon stuff above npc's heads
								oSetEnableBalloon(PresentationWorld, !Hide_UI_Panels); // And this should be the text bubble above their heads
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

					auto PresentationWorld = BNSClient_GetPresentationWorld();
					if (PresentationWorld) {
						oSetEnableNamePlate(PresentationWorld, !SHOW_NAMEPLATES);
						SHOW_NAMEPLATES = !SHOW_NAMEPLATES;
					}

					AddNotification(
						SHOW_NAMEPLATES ? L"Name Plates Visible" : L"<font name=\"00008130.UI.Label_DarkYellow_out_20\">Name Plates Hidden</font>", // Msg
						L"", // Particle Ref
						L"", // Sound
						0, // Track
						false, // Stop Previous Sound
						false, // Headline2
						false, // Boss Headline
						false, // Chat
						0, // Other Category type if none of the above (0 = Scrolling Text headline)
						L"" // Sound 2
					);
				}
				else if (ToggleNamePlates && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					ToggleNamePlates = false;

				break;
			case 0x56: // V Key will add again later
				if (!toggleFilter && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					toggleFilter = true;
				}
				else if (toggleFilter && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					toggleFilter = false;
				break;

			case 0x4C:
				if (!CombatLog && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					CombatLog = true;
					DWORD oldprotect;
					VirtualProtect((LPVOID)CombatLogMessageHandler, sizeof(combatlog_turnedoff), PAGE_EXECUTE_READWRITE, &oldprotect);

					std::wstring message;
					if (*reinterpret_cast<BYTE*>(CombatLogMessageHandler) == 0xC3) {
						message = L"<image enablescale=\"true\" imagesetpath=\"00009076.Duel\" scalerate=\"1.2\"/>Combat Log Enabled";
						memcpy((LPVOID)CombatLogMessageHandler, combatlog_original, sizeof(combatlog_original));
					}
					else {
						message = L"<image enablescale=\"true\" imagesetpath=\"00009076.Duel\" scalerate=\"1.2\"/><font name=\"00008130.UI.Label_Red02_20_out\">Combat Log Disabled</font>";
						memcpy((LPVOID)CombatLogMessageHandler, combatlog_turnedoff, sizeof(combatlog_turnedoff));
					}

					VirtualProtect((LPVOID)CombatLogMessageHandler, sizeof(combatlog_turnedoff), oldprotect, &oldprotect);

					AddNotification(
						message.c_str(), // Msg
						L"", // Particle Ref
						L"", // Sound
						0, // Track
						false, // Stop Previous Sound
						false, // Headline2
						false, // Boss Headline
						false, // Chat
						0, // Other Category type if none of the above (0 = Scrolling Text headline)
						L"" // Sound 2
					);
				}
				else if (CombatLog && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					CombatLog = false;
				break;

			case 0x54:
				if (!dText && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
					dText = true;
					bDisableText = !bDisableText;

					AddNotification(
						bDisableText ? L"<font name=\"00008130.UI.Label_Red02_20_out\">Text Parsing Disabled</font>" : L"<font name=\"00008130.UI.Label_DarkYellow_out_20\">Text Parsing Enabled</font>", // Msg
						L"", // Particle Ref
						L"", // Sound
						0, // Track
						true, // Stop Previous Sound
						false, // Headline2
						false, // Boss Headline
						false, // Chat
						0, // Other Category type if none of the above (0 = Scrolling Text headline)
						!bDisableText ? L"00003805.Signal_UI.S_Sys_VoiceChat_InCue" : L"00003805.Signal_UI.S_Sys_VoiceChat_OutCue" // Sound 2
					);
				}
				else if (dText && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
					dText = false;
				break;
		}
	}
	return oBInputKey(thisptr, InputKeyEvent);
}

char __fastcall hkBnsUIEngineInterfaceImpl_Show(IUIEngineInterface* Instance, unsigned int Id, bool bEvent) {
	if (Instance && Hide_UI_Panels) {
		auto panelName = Instance->vfptr->GetTag(Instance, Id);
		auto isToBehidden = std::any_of(PANEL_NAMES.begin(), PANEL_NAMES.end(), [panelName](std::wstring elem) {
			return wcscmp(elem.c_str(), panelName) == 0;
			});

		if (isToBehidden) {
			return 0;
		}
	}

	return BnsUIEngineInterfaceImpl_Show(Instance, Id, bEvent);
}

bool __fastcall hkFormatTextArgumentList_1(std::wstring* output, unsigned __int64 formatAlias, char* args) {
	if (bDisableText) {
		output[0] = L'\0';
		return output;
	}

	return oFormatTextArgumentList_1(output, formatAlias, args);
}

bool __fastcall hkFormatTextArgumentList_2(std::wstring* output, const wchar_t* formatAlias, char* args) {
	if (bDisableText) {
		output[0] = L'\0';
		return output;
	}

	return oFormatTextArgumentList_2(output, formatAlias, args);
}

void NTAPI DllNotification(ULONG notification_reason, const LDR_DLL_NOTIFICATION_DATA* notification_data, PVOID context)
{
	if (notification_reason == LDR_DLL_NOTIFICATION_REASON_LOADED) {
		//ConsoleWrite(L"DLL Loaded: %s\n", notification_data->Loaded.BaseDllName->Buffer);
		if (wcsncmp(notification_data->Loaded.BaseDllName->Buffer, xorstr_(L"wlanapi"), 7) == 0) {

			if (BNSClientInstance)
				BNSInstance = *(BNSClient**)BNSClientInstance;

			if (oUIEngineInterfaceGetInstance) {
				auto Instance = oUIEngineInterfaceGetInstance();

				DetourTransactionBegin();
				DetourUpdateThread(NtCurrentThread());

				BnsUIEngineInterfaceImpl_Show = (_BnsUIEngineInterfaceImpl_Show)&*Instance->vfptr->Show;
				DetourAttach(&(PVOID&)BnsUIEngineInterfaceImpl_Show, &hkBnsUIEngineInterfaceImpl_Show);
				DetourTransactionCommit();
			}
		}
	}
	return;
}

// Used this for debugging purposes
/*
* DeliveryGoodsOverConfirmPanel <-- filter that out
bool __fastcall hkUiStateGameShowPanel(__int64 UiStateGame, UIPanel* panel, bool show, bool playEvent, bool processPanelShowEvent) {
	if (panel != NULL)
		if(panel->panelName == NULL)
			printf("Panel ID: %d | Show: %d | playEvent: %d | processPanelShowEvent: %d\n", panel->panelId, show, playEvent, processPanelShowEvent);
		else
			printf("Panel: %S | Show: %d | playEvent: %d | processPanelShowEvent: %d\n", panel->panelName, show, playEvent, processPanelShowEvent);

	return oUiStateGameShowPanel(UiStateGame, panel, show, playEvent, processPanelShowEvent);
}
*/

bool __fastcall hkformatTextVariadicArguments_2(std::wstring* output, const wchar_t* formatAlias, ...) {
	va_list va;
	va_start(va, formatAlias);
	return (unsigned __int8)oFormatTextArgumentList_2(output, formatAlias, va);
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

		bool diffPattern = false;

		auto sBinput = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("0F B6 47 18 48 8D 4C 24 30 89 03")));
		if (sBinput != data.end()) {
			uintptr_t aBinput = (uintptr_t)&sBinput[0] - 0x38;
			oBInputKey = module->rva_to<std::remove_pointer_t<decltype(oBInputKey)>>(aBinput - handle);
			DetourAttach(&(PVOID&)oBInputKey, &hkBInputKey);
		}

		// Stops invokeGameMessage for combat log
		// Same as with FPSBooster, we stop this function from running as it does the lookup process and everything else before checking if combat log printing was disabled where as it should be done after
		// Unclear whether or not this provides an FPS bonus like it did with the UE3 client but still including it because who really uses the combat log.
		auto sMHandler = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("49 89 43 C0 4D 89 4B B8 4D 8B C8 41 B8 08 00 00 00")));
		if (sMHandler != data.end()) {
			CombatLogMessageHandler = (uintptr_t)&sMHandler[0] - 0x4B;
			memcpy((LPVOID)combatlog_original, (LPVOID)CombatLogMessageHandler, sizeof(combatlog_original));
			DWORD oldprotect;
			VirtualProtect((LPVOID)CombatLogMessageHandler, sizeof(combatlog_turnedoff), PAGE_EXECUTE_READWRITE, &oldprotect);
			memcpy((LPVOID)CombatLogMessageHandler, combatlog_turnedoff, sizeof(combatlog_turnedoff));
			VirtualProtect((LPVOID)CombatLogMessageHandler, sizeof(combatlog_turnedoff), oldprotect, &oldprotect);
		}

		auto sFormatTextArgumentList = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("4C 89 44 24 18 48 89 54 24 10 48 89 4C 24 08 48 83 EC 58 48 C7 44 24 30 FE FF FF FF 48 83 7C 24")));
		if (sFormatTextArgumentList != data.end()) {
			oFormatTextArgumentList_1 = module->rva_to<std::remove_pointer_t<decltype(oFormatTextArgumentList_1)>>((uintptr_t)&sFormatTextArgumentList[0] - handle);
			oFormatTextArgumentList_2 = module->rva_to<std::remove_pointer_t<decltype(oFormatTextArgumentList_2)>>((uintptr_t)&sFormatTextArgumentList[0] + 0x100 - handle);
			DetourAttach(&(PVOID&)oFormatTextArgumentList_1, &hkFormatTextArgumentList_1);
			DetourAttach(&(PVOID&)oFormatTextArgumentList_2, &hkFormatTextArgumentList_2);
		}

		auto sShowPanel = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 41 0F B6 F9 41 0F B6 F0 48 8B")));
		if (sShowPanel != data.end()) {
			//std::cout << "Found UiStateGame::showPanel" << std::endl;
			oUiStateGameShowPanel = module->rva_to<std::remove_pointer_t<decltype(oUiStateGameShowPanel)>>((uintptr_t)&sShowPanel[0] - handle);
			//DetourAttach(&(PVOID&)oUiStateGameShowPanel, &hkUiStateGameShowPanel);
		}

		// Used for sending notifications about certain actions
		auto addInstantNotif = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("45 33 DB 41 8D 42 ?? 3C 02 BB 05 00 00 00 41 0F 47 DB")));
		if (addInstantNotif == data.end()) {
			// Old compiler stuff (NAEU CLIENT)
			diffPattern = true;
			addInstantNotif = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 FF 80 BC 24 80 00 00 00 01 75 05")));
		}

		if (addInstantNotif != data.end()) {
			oAddInstantNotification = module->rva_to<std::remove_pointer_t<decltype(oAddInstantNotification)>>((uintptr_t)&addInstantNotif[0] - (diffPattern ? 0x13 : 0x68) - handle);
		}

		diffPattern = false;

		auto result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("66 89 54 24 10 48 89 4C 24 08 57 48 81 EC 10 02 00 00 48 C7 84 24 B8 00 00 00 FE FF FF FF")));
		if (result != data.end()) {
			BNSClient_GetWorld = module->rva_to<std::remove_pointer_t<decltype(BNSClient_GetWorld)>>(GetAddress(((uintptr_t)&result[0] + 0x38), 1, 5) - handle);
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to find BNSClient::GetWorld"), xorstr_(L"[customhud] Search Error"), MB_OK);

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B 05 ?? ?? ?? ?? 48 85 C0 74 08 48 8B 80 ?? ?? 00 00 C3 C3 CC CC CC CC CC CC CC CC CC CC CC 48 8D 05 ?? ?? ?? ?? C7 41 08 00 00 00 00 48 89 01 C3")));
		if (result != data.end()) {
			BNSClient_GetPresentationWorld = module->rva_to<std::remove_pointer_t<decltype(BNSClient_GetPresentationWorld)>>((uintptr_t)&result[0] - handle);
		}
		else {
			BNSClient_GetPresentationWorld = &GetPresentationWorld;

			result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("0F 29 70 C8 ?? 8B F2 48 8B ?? 48 83 79 08 00")));
			if (result != data.end()) {
				BNSClientInstance = (uintptr_t*)GetAddress((uintptr_t)&result[0] + 0x15, 3, 7);
			}
		}

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 43 08 66 C7 83 ?? ?? 00 00 01 00 33 D2 89 93 ?? ?? 00 00 88 93 ?? ?? 00 00 48 89 93 ?? ?? 00 00 48 89 93 ?? ??")));
		if (result == data.end()) {
			result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 83 EC 48 48 C7 44 24 38 FE FF FF FF 48 83 3D ?? ?? ?? ?? 00 75 ?? B9 F0 14 00 00")));
			diffPattern = true;
		}

		if (result != data.end()) {

			if (diffPattern) {
				oUIEngineInterfaceGetInstance = module->rva_to<std::remove_pointer_t<decltype(oUIEngineInterfaceGetInstance)>>((uintptr_t)&result[0] - handle);
			}
			else {
				if (*reinterpret_cast<BYTE*>(&result[0] - 0x53) == 0x40) {
					oUIEngineInterfaceGetInstance = module->rva_to<std::remove_pointer_t<decltype(oUIEngineInterfaceGetInstance)>>((uintptr_t)&result[0] - 0x53 - handle);
					UIEngineInterfacePtr = (uintptr_t*)GetAddress((uintptr_t)&result[0] - 0x44, 3, 7);
				}
				else
					oUIEngineInterfaceGetInstance = module->rva_to<std::remove_pointer_t<decltype(oUIEngineInterfaceGetInstance)>>((uintptr_t)&result[0] - 0x6F - handle);
			}
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to get UIEngineInterfaceImplInstance"), xorstr_(L"[customhud] Search Error"), MB_OK);

		/*
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B 01 45 33 C0 BA ?? 14 00 00 FF 50 10 48 8B D8 48 89 44 24 40")));
		if (result != data.end())
			oUIEngineInterfaceGetInstance = module->rva_to<std::remove_pointer_t<decltype(oUIEngineInterfaceGetInstance)>>((uintptr_t)&result[0] - 0x37 - handle);
		*/

		// 48 89 5C 24 30 48 89 7C 24 38 88 91 A0 00 00 00 - 0x1C (SetEnableNamePlate)
		// 48 85 D2 0F B6 D3 48 0F 44 CD E8 (End of BUiWorld::ShowHud)
		auto sSetNamePlate = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 85 C0 74 73 48 8B 88 B8 00 00 00 48 85 C9 74 4A 33 D2")));
		if (sSetNamePlate != data.end()) {
			uintptr_t aSetNamePlate = (uintptr_t)&sSetNamePlate[0] + 0x13;
			uintptr_t aSetBalloon = (uintptr_t)&sSetNamePlate[0] + 0x4F;
			uintptr_t aSetIndicator = (uintptr_t)&sSetNamePlate[0] + 0x32;

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
	static PVOID cookie;
	if (tLdrRegisterDllNotification LdrRegisterDllNotification = reinterpret_cast<tLdrRegisterDllNotification>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrRegisterDllNotification")))
		LdrRegisterDllNotification(0, DllNotification, NULL, &cookie); //Set a callback for when Dll's are loaded/unloaded
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