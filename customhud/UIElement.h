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




struct GameSession {
	char pad[0x98];
	float receivedServerPing;
};

struct BNSClient {
	char pad[0x48];
	GameSession* game;
	char pad2[0x50];
	uintptr_t* GameWorld;
	char pad3[0x10];
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
static_assert(offsetof(BNSClient, PresentationWorld) == 0xB8);

template<class T>
class TArray
{
	friend class FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline int Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline const T& operator[](int i) const
	{
		return Data[i];
	};

	inline bool IsValidIndex(int i) const
	{
		return i < Num();
	}

private:
	T* Data;
	int Count;
	int Max;
};

struct FNameEntryId
{
	unsigned int Value;
};

struct FName
{
	FNameEntryId ComparisonIndex;
	unsigned int Number;
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		const auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

struct SWidget
{
	char pad_0000[0x148];
	int LayerId;
	int OutGoingLayerId;
	char pad_0150[0x58];
	char Visibility;
	char pad[0x17];
	float RenderOpacity;
	char pad_01C8[0x54];
	FName TypeOfWidget;
	FName Tag;
};

static_assert(offsetof(SWidget, LayerId) == 0x148);
static_assert(offsetof(SWidget, OutGoingLayerId) == 0x14C);
static_assert(offsetof(SWidget, Visibility) == 0x1A8);
static_assert(offsetof(SWidget, pad) == 0x1A9);
static_assert(offsetof(SWidget, RenderOpacity) == 0x1C0);
static_assert(offsetof(SWidget, TypeOfWidget) == 0x218);
