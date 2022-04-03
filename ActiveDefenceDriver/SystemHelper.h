#pragma once
#include<fltKernel.h>
#include <ntddk.h>

//OS version
#define OS_VERSION_ERROR		0
#define OS_VERSION_2000			1
#define OS_VERSION_XP			2
#define OS_VERSION_SERVER_2003	3
#define OS_VERSION_VISTA		4
#define OS_VERSION_VISTA_SP1	5
#define OS_VERSION_VISTA_SP2	6
#define OS_VERSION_WIN7			7
#define OS_VERSION_WIN7_SP1		8

//System information
typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation, //Basic Infomation
	SystemProcessorInformation,  //The number of processors
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemNotImplemented1,
	SystemProcessesAndThreadsInformation, //Process infomation and resource of process: handles pages
	SystemCallCounts,
	SystemConfigurationInformation,
	SystemProcessorTimes,
	SystemGlobalFlag,
	SystemNotImplemented2,
	SystemModuleInformation,
	SystemLockInformation,
	SystemNotImplemented3,
	SystemNotImplemented4,
	SystemNotImplemented5,
	SystemHandleInformation, 
	SystemObjectInformation,
	SystemPagefileInformation,
	SystemInstructionEmulationCounts,
	SystemInvalidInfoClass1,
	SystemCacheInformation,
	SystemPoolTagInformation,
	SystemProcessorStatistics,
	SystemDpcInformation,
	SystemNotImplemented6,
	SystemLoadImage, //Image
	SystemUnloadImage,
	SystemTimeAdjustment,
	SystemNotImplemented7,
	SystemNotImplemented8,
	SystemNotImplemented9,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemLoadAndCallImage,
	SystemPrioritySeparation,
	SystemNotImplemented10,
	SystemNotImplemented11,
	SystemInvalidInfoClass2,
	SystemInvalidInfoClass3,
	SystemTimeZoneInformation,
	SystemLookasideInformation,
	SystemSetTimeSlipEvent,
	SystemCreateSession,
	SystemDeleteSession,
	SystemInvalidInfoClass4,
	SystemRangeStartInformation,
	SystemVerifierInformation,
	SystemAddVerifier,
	SystemSessionProcessesInformation
}SYSTEM_INFORMATION_CLASS;

//IDT 
#pragma pack(1)
typedef struct _IDT_INFORMATION
{
	USHORT IdtLimit;
	USHORT LowIdtBase;
	USHORT HighIdtBase;
} IDT_INFORMATION, * PIDT_INFORMATION, ** PPIDT_INFORMATION;
#pragma pack()

#pragma pack(1)
typedef struct _IDT_ENTRY {
	USHORT LowOffset;
	USHORT Selector;
	UCHAR  unused_lo;
	UCHAR  SegmentType : 4;
	UCHAR  SystemSegmentFlag : 1;
	UCHAR  DPL : 2;
	UCHAR  P : 1;
	USHORT HighOffset;
}IDT_ENTRY, * PIDT_ENTRY, ** PPIDT_ENTRY;
#pragma pack()

typedef struct _SYSTEM_MODULE_ENTRY
{
	ULONG Unknow0[2];
	PVOID ModuleBase;
	ULONG ModuleSize;
	ULONG Flags;
	ULONG64 Unknow1;
	UCHAR   ModuleName[256];
}SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG               NumberOfModules;
	SYSTEM_MODULE_ENTRY Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

//Function define
NTSTATUS
NTAPI
ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS			SystemInformationClass,
	OUT PVOID							SystemInformation,
	IN ULONG							SystemInformationLength,
	OUT	PULONG							ReturnLength
);

NTSTATUS
NTAPI
ZwQueryInformationProcess(
	IN	HANDLE							ProcessHandle,
	IN	PROCESSINFOCLASS				ProcessInformationClass,
	OUT PVOID							ProcessInformation,
	IN	ULONG							ProcessInformationLength,
	OUT	PULONG							ReturnLength
);

PMDL MakeAddressWritable(IN ULONG VirtuallAddress, IN ULONG AddressSize, OUT PVOID ReflectAddress);
PUNICODE_STRING GetProcessNameByEprocess(IN PEPROCESS Eprocess);
BOOLEAN InitCommonVariables();
PSYSTEM_MODULE_INFORMATION GetSystemModule();
ULONG GetKiSystemAddress();
BOOLEAN GetSystemModuleByName(IN char* ModuleName, OUT PSYSTEM_MODULE_ENTRY SystemModuleEntry);
BOOLEAN GetShadowSsdtInfo(IN PULONG ServiceTableBase, IN ULONG ServiceNumber);