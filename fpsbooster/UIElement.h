/*
class UIElement
{
public:
	char pad_0000[0x158]; //0x0000
	int ID; //0x0148
	char pad_014C[0x1D8]; //0x014C
	float Red;
	float Green;
	float Blue;
	float Alpha;
};

struct BLUEPRINT_ORIGINAL {
	int8_t pad_158[0x158];
	int ID;
};

struct BLUEPRINT_ELEMENT_CONTROL
{
	int8_t pad_2D0[0x2D0];
	BLUEPRINT_ORIGINAL* Struct;
	int8_t pad_328[0x50];
	float Red;
	float Green;
	float Blue;
	float Alpha;
};
*/

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
	char padd[0xB8];
	uintptr_t* PresentationWorld;
};

// my sanity
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
