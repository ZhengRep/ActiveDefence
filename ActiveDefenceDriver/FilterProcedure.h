#pragma once
#include "GlobalVarible.h"

NTSTATUS PostNtProceduer(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject);
NTSTATUS PreviousNtDeleteFile(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject);
NTSTATUS PreviousNtTerminateProcess(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject);