class UIPanel {

public:
	enum class Panel_Visibility {
		NONE = 0,
		SHOW = 1,
		HIDE = 2
	};
	unsigned __int64* baseclass;
	__int8 panelId;
	char pad_02[0x2];
	const wchar_t* panelName;
	char pad_50[0x38];
	Panel_Visibility visibleState;
	char activated;
};

class SCompoundWidget {
public:
	char paddmeupdad[0x148];
	int LayerID;
	char padddd[0xC];
	float HSize;
	float VSize;
	char padmeupagain[0x48];
	char IsVisible;
	char again[0xBF];
	SCompoundWidget* Parent; // 0x268
	char morepadding[0xB8];
	float Red;
	float Green;
	float Blue;
	float AlphaLevel; // 0x334
};

class BInstance {
public:
	char pad[0xA0];
	uintptr_t* GameWorld;
	char padd[0x10];
	uintptr_t* PresentationWorld;
};

// my sanity (If you don't know what this is then go read about it)
static_assert(offsetof(UIPanel, panelId) == 0x8);
static_assert(offsetof(UIPanel, panelName) == 0x10);
static_assert(offsetof(UIPanel, visibleState) == 0x50);
static_assert(offsetof(UIPanel, activated) == 0x54);
static_assert(offsetof(SCompoundWidget, LayerID) == 0x148);
static_assert(offsetof(SCompoundWidget, IsVisible) == 0x1A8);
static_assert(offsetof(SCompoundWidget, Parent) == 0x268);
static_assert(offsetof(SCompoundWidget, HSize) == 0x158);
static_assert(offsetof(SCompoundWidget, VSize) == 0x15C);
static_assert(offsetof(SCompoundWidget, AlphaLevel) == 0x334);
static_assert(offsetof(BInstance, PresentationWorld) == 0xB8);