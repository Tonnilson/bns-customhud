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
_PresentationWorldSetNamePlate oSetEnableIndicator;
_PresentationWorldSetNamePlate oSetEnableBalloon;

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

bool(__fastcall* oformatTextVariadicArguments_1)(std::wstring* output, const unsigned __int64 textDataId, ...);
bool __fastcall hkformatTextVariadicArguments_1(std::wstring* output, const unsigned __int64 textDataId, ...);

bool(__fastcall* oformatTextVariadicArguments_2)(std::wstring* output, const wchar_t* formatAlias, ...);
bool __fastcall hkformatTextVariadicArguments_2(std::wstring* output, const wchar_t* formatAlias, ...);
bool(__fastcall* oFormatTextArgumentList_2)(std::wstring* output, const wchar_t* formatAlias, char* args);
bool __fastcall hkFormatTextArgumentList_2(std::wstring* output, const wchar_t* formatAlias, char* args);

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length)
{
    if (!AddressOfCall)
        return 0;

    long delta = *(long*)(AddressOfCall + index);
    return (AddressOfCall + delta + length);
}

// Notification Function
typedef void(__cdecl* _AddInstantNotification)(
    uintptr_t* thisptr, // World Object
    const wchar_t* text, // Text, DUR
    const wchar_t* particleRef, // No fucking idea what it does
    const wchar_t* sound, // Pretty sure takes old upk indexing path (xxxxxxxx.EntryName)
    char track, // Audio track related
    bool stopPreviousSound, // Says whether a sound from previous notice should end (duh)
    bool headline2, // Really weird text that is big, red and lasts like 20-30 seconds on screen...
    bool boss_headline, // Should be no brainer
    bool chat,
    char category,
    const wchar_t* sound2 // Uhh?
);
_AddInstantNotification oAddInstantNotification;

/*
Including as a source ref
Roughly what it looks like granted this is from Frontier Build
void __fastcall World::AddInstantNotification(
        World *this,
        const wchar_t *text,
        const wchar_t *particleRef,
        const wchar_t *sound,
        char track,
        bool stopPreviousSound,
        bool headline2,
        bool boss_headline,
        bool chat,
        char category,
        const wchar_t *sound2)
{
  int v11; // ebx
  __int64 v12; // rax
  GameMessageHistory *v13; // rcx
  BUiWorld *v14; // rcx
  NetArenaEventHandlerVtbl *vfptr; // rax
  WarningNotificationPanel *CreateArenaLobbyRoomNotify; // rbx
  InstantNotificationPanel *ProcessInfoGateNetObjectEvent; // rcx
  HeadlineFxPanel *EnterArenaLobby; // rcx
  NetInfoGateEventHandlerVtbl *v19; // rax
  ClientEventHandlerVtbl *v20; // rcx

  if ( headline2 )
  {
    v11 = 2;
  }
  else if ( boss_headline )
  {
    v11 = 3;
  }
  else
  {
    if ( !particleRef )
      goto LABEL_10;
    v12 = -1i64;
    do
      ++v12;
    while ( particleRef[v12] );
    if ( v12 )
    {
      v11 = 1;
    }
    else
    {
LABEL_10:
      if ( chat )
      {
        v11 = 4;
      }
      else
      {
        v11 = 5;
        if ( (unsigned __int8)(category - 12) > 2u )
          v11 = 0;
      }
    }
  }
  v13 = this->_gameMessageHistory[v11];
  if ( v13 )
    GameMessageHistory::PushGameMessage(v13, text, particleRef, sound, track, stopPreviousSound, category, sound2);
  if ( *(_QWORD *)&BNSClient::instance )
  {
    v14 = *(BUiWorld **)(*(_QWORD *)&BNSClient::instance + 200i64);
    if ( v14 )
    {
      vfptr = v14->NetArenaEventHandler::vfptr;
      if ( !vfptr )
      {
        v19 = v14->NetInfoGateEventHandler::vfptr;
        if ( v19 )
        {
          if ( v11 )
          {
            if ( v11 != 5 )
              return;
            ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)v19[4].ProcessInfoGateNetObjectEvent;
          }
          else
          {
            ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)v19[5].__vecDelDtor;
          }
        }
        else
        {
          v20 = v14->ClientEventHandler::vfptr;
          if ( !v20 )
            return;
          if ( v11 )
          {
            if ( v11 != 5 )
              return;
            ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)v20[1190].ProcessClientEvent;
          }
          else
          {
            ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)v20[1190].__vecDelDtor;
          }
        }
        goto LABEL_45;
      }
      switch ( v11 )
      {
        case 4:
          if ( !BYTE3(vfptr[14908].CreateArenaLobbyRoomNotify) )
            BUiWorld::popChatGameMessage(v14);
          return;
        case 0:
        case 3:
          ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)vfptr[44].__vecDelDtor;
          goto LABEL_45;
        case 2:
          CreateArenaLobbyRoomNotify = (WarningNotificationPanel *)vfptr[44].CreateArenaLobbyRoomNotify;
          if ( CreateArenaLobbyRoomNotify && CreateArenaLobbyRoomNotify->_running )
          {
            std::deque<WarningNotificationPanel::BufferedMessage>::_Tidy(&CreateArenaLobbyRoomNotify->_messageLines);
            WarningNotificationPanel::fetchGameMessageIfPossible(CreateArenaLobbyRoomNotify);
            WarningNotificationPanel::updateWidgets(CreateArenaLobbyRoomNotify);
          }
          return;
        case 5:
          ProcessInfoGateNetObjectEvent = (InstantNotificationPanel *)vfptr[44].PrepareEnterArenaLobby;
LABEL_45:
          if ( ProcessInfoGateNetObjectEvent && ProcessInfoGateNetObjectEvent->_running )
            InstantNotificationPanel::processTextMessage(ProcessInfoGateNetObjectEvent);
          return;
      }
      EnterArenaLobby = (HeadlineFxPanel *)vfptr[44].EnterArenaLobby;
      if ( EnterArenaLobby && EnterArenaLobby->_running && !EnterArenaLobby->_particleMsgPlaying )
        HeadlineFxPanel::processParticleMessage(EnterArenaLobby);
    }
  }
}
*/