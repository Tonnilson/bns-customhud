#pragma once

class BInputKey {
public:
    int Key;
    bool bCtrlPressed;
    bool bShiftPressed;
    bool bAltPressed;
    bool bDoubleClicked;
    bool bTpsModeKey;
};

enum class EngineKeyStateType {
    EKS_PRESSED = 0,
    EKS_RELEASED = 1,
    EKS_REPEAT = 2,
    EKS_DOUBLECLICK = 3,
    EKS_AXIS = 4
};

class EInputKeyEvent {
public:
    char padding[0x18];
    char _vKey;
    char padd_2[0x2];
    EngineKeyStateType KeyState;
    bool bCtrlPressed;
    bool bShiftPressed;
    bool bAltPressed;
};

bool(__fastcall* oBInputKey)(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent);
bool __fastcall hkBInputKey(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent);

uintptr_t* BNSClientInstance = NULL;
BInstance* BNSInstance;

// Define our pointer
__int64 UiStateGamePtr = 0;
SCompoundWidget* TargetedLayer = NULL;

// Really do I have to explain these?
typedef __int64(__cdecl* _UIPanelIsVisible)(UIPanel* thisptr, bool includedParents);
_UIPanelIsVisible oUIPanelIsVisible;

typedef unsigned __int8(__cdecl* _UiStateGameshowPanel)(__int64 UiStateGame, UIPanel* panel, bool show, bool playEvent, bool processPanelShowEvent);
_UiStateGameshowPanel oUiStateGameShowPanel;

// Similar to ShowPanel but shows or hides based on previous visibility state
typedef void(__cdecl* _UiPanelToggle)(UIPanel* thisptr, bool playEvent);
_UiPanelToggle oUIPanelToggle;

typedef void(__cdecl* _PresentationWorldSetNamePlate)(uintptr_t *pWorld, bool enable);
_PresentationWorldSetNamePlate oSetEnableNamePlate;

/*
/// <summary>
/// Show UI Panel
/// </summary>
bool(__fastcall* oUiStateGameShowPanel)(__int64 UiStateGame, UIPanel* panel, bool show, bool playEvent, bool processPanelShowEvent);
bool __fastcall hkUiStateGameShowPanel(__int64 UiStateGame, UIPanel* panel, bool show, bool playEvent, bool processPanelShowEvent);
*/

// Called when entering world, we hook this to get the UiStateGame pointer reliably
bool(__fastcall* oUiStateGame)(__int64);
bool __fastcall hkUiStateGame(__int64 thisptr);

// Sets name of UIPanel, called when panel is created
bool(__fastcall* oUIPanelSetName)(UIPanel*, const wchar_t*);
bool __fastcall hkUIPanelSetName(UIPanel* uiPanel, const wchar_t* name);

// Using this to get rid of a layer I hate.
bool(__fastcall* oSCompoundWidgetOnPaint)(SCompoundWidget* thisptr, __int64 Args, __int64 Allotedgeometry, __int64 MyCullingRect, __int64 OutDrawElements, int LayerID, __int64 InWidgetStyle, bool bParentEnabled);
bool __fastcall hkSCompoundWidgetOnPaint(SCompoundWidget* thisptr, __int64 Args, __int64 Allotedgeometry, __int64 MyCullingRect, __int64 OutDrawElements, int LayerID, __int64 InWidgetStyle, bool bParentEnabled);

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length)
{
    if (!AddressOfCall)
        return 0;

    long delta = *(long*)(AddressOfCall + index);
    return (AddressOfCall + delta + length);
}
