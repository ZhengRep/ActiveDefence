#pragma once
#include<fltKernel.h>
#include "HookHelper.h"

//HookApproach relative
#define HOOK_OBJECTS 1024

extern HOOK_OBJECT __NtDeleteFile;
extern HOOK_OBJECT __NtTerminateProcess;

PHOOK_OBJECT __FakeSsdt[HOOK_OBJECTS] = { 0 };
PHOOK_OBJECT __FakeSssdt[HOOK_OBJECTS] = { 0 };

//HookAPI relative
ULONG __MmUserProbeAddress = 0;
ULONG __KiSystemService = 0;
ULONG __KeServiceDescriptorTable = 0;
ULONG __ServiceTableBase = 0;
ULONG __NumberOfServices = 0;
ULONG __ShadowServiceTableBase = 0;
ULONG __ShadowNumberOfServices = 0;
ULONG __HookPosition = 0;
ULONG __JmpBackPosition = 0;
ULONG __NtdllBase = 0;
ULONG __NtdllSize = 0;
ULONG __NtUserSetWindowsHookExService = 0;

//Function pointer
typedef NTSTATUS(*LPFN_FILTERPROCEDURE)(PCONTEXT_OBJECT ContextObject, PHOOK_OBJECT HookObject);
typedef NTSTATUS(*LPFN_ORIGINALPROCEDURE1)(ULONG);
typedef NTSTATUS(*LPFN_ORIGINALPROCEDURE2)(ULONG, ULONG);