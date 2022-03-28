#include "FackSSDT.h"
#include "HookHelper.h"
#include "GlobalVarible.h"

NTSTATUS FakeNtDeleteFile(IN POBJECT_ATTRIBUTES ObejectAttributes)
{
    NTSTATUS status = STATUS_SUCCESS;
	
	CONTEXT_OBJECT ContextObject;
	ContextObject.ParameterCount = 1;
	ContextObject.ParameterData1 = (ULONG)ObejectAttributes;

	InterlockedIncrement((volatile long *)&(__NtDeleteFile.UseReference));

	if (__NtDeleteFile.FilterType & FILTER_TYPE_PREVIOUS)
	{
		status = ((LPFN_FILTERPROCEDURE)__NtDeleteFile.PreviousFilterProcedure)(&ContextObject, &__NtDeleteFile);
	}
	if (NT_SUCCESS(status))
	{
		status = ((LPFN_ORIGINALPROCEDURE1)__NtDeleteFile.OriginalFunctionAddress)(ContextObject.ParameterData1);
		if (NT_SUCCESS(status) && (__NtDeleteFile.FilterType & FILTER_TYPE_POST))
		{
			status = ((LPFN_FILTERPROCEDURE)__NtDeleteFile.PostFilterProcedure)(&ContextObject, &__NtDeleteFile);
		}
	}

	InterlockedDecrement((volatile long *)&(__NtDeleteFile.UseReference));

    return status;
}

NTSTATUS FakeNtTerminateProcess(IN HANDLE ProcessHandle, IN NTSTATUS ExitStatus)
{
	NTSTATUS status = STATUS_SUCCESS;

	CONTEXT_OBJECT ContextObject;
	ContextObject.ParameterCount = 2;
	ContextObject.ParameterData1 = (ULONG)ProcessHandle;
	ContextObject.ParameterData2 = (ULONG)ExitStatus;

	InterlockedIncrement((volatile long*)&(__NtTerminateProcess.UseReference));

	if (__NtTerminateProcess.FilterType & FILTER_TYPE_PREVIOUS)
	{
		status = ((LPFN_FILTERPROCEDURE)__NtTerminateProcess.PreviousFilterProcedure)(&ContextObject, &__NtTerminateProcess);
	}
	if (NT_SUCCESS(status))
	{
		status = ((LPFN_ORIGINALPROCEDURE2)__NtTerminateProcess.OriginalFunctionAddress)(ContextObject.ParameterData1, ContextObject.ParameterData2);
		if (NT_SUCCESS(status) && (__NtTerminateProcess.FilterType & FILTER_TYPE_POST))
		{
			status = ((LPFN_FILTERPROCEDURE)__NtTerminateProcess.PostFilterProcedure)(&ContextObject, &__NtTerminateProcess);
		}
	}

	InterlockedDecrement((volatile long*)&(__NtTerminateProcess.UseReference));

	return status;
}
