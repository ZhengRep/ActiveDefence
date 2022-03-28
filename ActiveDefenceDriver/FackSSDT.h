#pragma once
#include<fltKernel.h>

NTSTATUS FakeNtDeleteFile(IN POBJECT_ATTRIBUTES ObejectAttributes);
NTSTATUS FakeNtTerminateProcess(IN HANDLE ProcessHandle, IN NTSTATUS ExitStatus);