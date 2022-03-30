#pragma once
#include<fltKernel.h>

//Filter Type
#define FILTER_TYPE_NONE		0x0
#define FILTER_TYPE_PREVIOUS	0x1
#define FILTER_TYPE_POST		0x2
#define FILTER_TYPE_USER		0x4
#define FILTER_TYPE_KERNEL		0x8

//Define Crime
#define CRIME_MINOR_NTDELETEFILE				0x10000003
#define CRIME_MINOR_NTTERMINATEPROCESS			0x40000007

//Record Parameter
typedef struct _CONTEXT_OBJECT { 
	ULONG ParameterCount;
	ULONG ParameterData1;
	ULONG ParameterData2;
	ULONG ParameterData3;
	ULONG ParameterData4;
	ULONG ParameterData5;
	ULONG ParameterData6;
	ULONG ParameterData7;
	ULONG ParameterData8;
	ULONG ParameterData9;
	ULONG ParameterData10;
	ULONG ParameterData11;
	ULONG ParameterData12;
	ULONG ParameterData13;
	ULONG ParameterData14;
	ULONG ParameterData15;
	ULONG ParameterData16;
}CONTEXT_OBJECT, * PCONTEXT_OBJECT, ** PPCONTEXT_OBJECT;

typedef struct _HOOK_OBJECT {
	ULONG	FunctionService;
	ULONG	OriginalFunctionAddress;
	ULONG	FakeFunctionAddress;
	ULONG	PreviousFilterProcedure;
	ULONG	PostFilterProcedure;
	ULONG	FilterType;
	ULONG   CrimeType;
	ULONG	UseReference;
} HOOK_OBJECT, * PHOOK_OBJECT, ** PPHOOK_OBJECT;

/*
* To watch api
 NtCreateFile
 NtOpenFile
 NtDeleteFile
 NtSetInformationFile
 NtLoadDriver
 NtUnloadDriver
 NtSetSystemInformation
 NtCreateThread
 NtOpenThread
 NtOpenProcess
 NtSuspendThread
 NtSuspendProcess
 NtGetContextThread
 NtSetContextThread
 NtTerminateThread
 NtAssignProcessToJobObject
 NtOpenSection
 NtCreateSymbolicLinkObject
 NtReadVirtualMemory
 NtWriteVirtualMemory
 NtProtectVirtualMemory
 NtSystemDebugControl
 NtDuplicateObject

*/
//Function define
ULONG			FindHookPosition();
BOOLEAN			Hook(IN ULONG HookPosition, IN ULONG Tampoline);
VOID			DpcProcedure();
BOOLEAN			UnHook();
VOID			FiFastCallEntry();
ULONG __stdcall Fake911(ULONG FunctionService, ULONG FunctionAddress, ULONG ServiceTableBase);
BOOLEAN			InitializeSysCallTable();
BOOLEAN			IsMajorProtect(IN ULONG CrimeType);
BOOLEAN			IsAllHookObjectIsUsing();