struct IUIEngineInterface;
struct BnsUIEngineInterfaceImpl;

struct IUIEngineInterfaceVtbl
{
	int(__fastcall* Load)(IUIEngineInterface* thisptr, const wchar_t*, const wchar_t*, int, const wchar_t*);
	char pad_0008[0x22];
	char(__fastcall* Show)(IUIEngineInterface* thisptr, unsigned int Id, bool bEvent);
	char(__fastcall* Hide)(IUIEngineInterface* thisptr, unsigned int Id, bool bEvent);
	char pad_0040[0x30];
	unsigned int(__fastcall* GetId)(IUIEngineInterface* thisptr, const wchar_t* szTag);
	const wchar_t* (__fastcall* GetTag)(IUIEngineInterface* thisptr, unsigned int Id);
	char gap[0xF30];
	void(__fastcall* SetEnableWindowClipBoard)(IUIEngineInterface* thisptr, bool);
};

static_assert(offsetof(IUIEngineInterfaceVtbl, Show) == 0x30);
static_assert(offsetof(IUIEngineInterfaceVtbl, Hide) == 0x38);
static_assert(offsetof(IUIEngineInterfaceVtbl, GetId) == 0x70);
static_assert(offsetof(IUIEngineInterfaceVtbl, GetTag) == 0x78);
static_assert(offsetof(IUIEngineInterfaceVtbl, SetEnableWindowClipBoard) == 0xFB0);

struct IUIEngineInterface
{
	IUIEngineInterfaceVtbl* vfptr;
};

IUIEngineInterface* UIEngineInterfaceInstance;
IUIEngineInterface* (__fastcall* oUIEngineInterfaceGetInstance)();

typedef char(__fastcall* _BnsUIEngineInterfaceImpl_Hide)(IUIEngineInterface* thisptr, unsigned int Id, bool bEvent);
_BnsUIEngineInterfaceImpl_Hide BnsUIEngineInterfaceImpl_Hide;
typedef char(__fastcall* _BnsUIEngineInterfaceImpl_Show)(IUIEngineInterface* thisptr, unsigned int Id, bool bEvent);
_BnsUIEngineInterfaceImpl_Show BnsUIEngineInterfaceImpl_Show;