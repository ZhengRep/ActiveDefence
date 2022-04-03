#pragma once
#include<fltKernel.h>
#include "HookHelper.h"

//HookApproach relative
#define HOOK_OBJECTS		1024
#define ALLOCATE_TAG		'spiH'

#define MEMOERY_USER_MODE		0x4
#define MEMOERY_KERNEL_MODE		0x8

extern NPAGED_LOOKASIDE_LIST __EventDataNpagedLookasideList;
LIST_ENTRY					__EventDataListHead;

extern HOOK_OBJECT			__NtDeleteFile;
extern HOOK_OBJECT			__NtTerminateProcess;

//Process relative
extern ULONG				__MajorProtectedMask = 0;
PEPROCESS					__OwnProcess = NULL;
extern ULONG				__BuildNumber;
UCHAR						__RecordPosition[5];
UCHAR						__SignatureData[5] = { 0x2b,0xe1,0xc1,0xe9,0x02 };
BOOLEAN						__Ready;

PHOOK_OBJECT				__FakeSsdt[HOOK_OBJECTS] = { 0 };
PHOOK_OBJECT				__FakeSssdt[HOOK_OBJECTS] = { 0 };

//HookAPI relative
ULONG						__MmUserProbeAddress = 0;
ULONG						__KiSystemService = 0;
ULONG						__KeServiceDescriptorTable = 0;
ULONG						__ServiceTableBase = 0;
ULONG						__NumberOfServices = 0;
ULONG						__ShadowServiceTableBase = 0;
ULONG						__ShadowNumberOfServices = 0;
ULONG						__HookPosition = 0;
ULONG						__JmpBackPosition = 0;
ULONG						__NtdllBase = 0;
ULONG						__NtdllSize = 0;
ULONG						__NtUserSetWindowsHookExService = 0;

//Event type
#define CRIME_MAJOR_MASK		0xF0000000
#define CRIME_MAJOR_PROCESS		0x40000000
#define CRIME_MAJOR_FILE		0x10000000
#define CRIME_MAJOR_REGISTRY	0x20000000
#define CRIME_MAJOR_SYS			0x80000000
#define CRIME_MAJOR_ALL			(CRIME_MAJOR_PROCESS | CRIME_MAJOR_FILE | CRIME_MAJOR_REGISTRY | CRIME_MAJOR_SYS)

#define NODE_TYPE_WHITE			0
#define NODE_TYPE_BLACK			1

#define JUDGMENT_REFUSE			1
#define JUDGMENT_ACCEPT			2
#define JUDGMENT_ALWAYS			4

//Function pointer
typedef NTSTATUS(*LPFN_FILTERPROCEDURE)(PCONTEXT_OBJECT ContextObject, PHOOK_OBJECT HookObject);
typedef NTSTATUS(*LPFN_ORIGINALPROCEDURE1)(ULONG);
typedef NTSTATUS(*LPFN_ORIGINALPROCEDURE2)(ULONG, ULONG);
