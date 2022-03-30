#include "FilterProcedure.h"
#include "SystemHelper.h"
#include "EventHelper.h"

NTSTATUS PostNtProceduer(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject)
{
	PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;

    return status;
}

NTSTATUS PreviousNtDeleteFile(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject)
{
	PAGED_CODE();

	NTSTATUS status = STATUS_SUCCESS;

	return status;
}

NTSTATUS PreviousNtTerminateProcess(PHOOK_OBJECT HookObject, PCONTEXT_OBJECT ContextObject)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	__try
	{
		HANDLE ProcessHandle = (HANDLE)ContextObject->ParameterData1;

		PEPROCESS EprocessOfCriminal;
		status = ObReferenceObjectByHandle(ProcessHandle, 0, NULL, KernelMode, (PVOID*)(&EprocessOfCriminal), NULL);
		if (!NT_SUCCESS(status))
		{
			return STATUS_UNSUCCESSFUL;
		}
		ObDereferenceObject(ProcessHandle);
		PUNICODE_STRING CriminalProcessName = GetProcessNameByEprocess(EprocessOfCriminal);

		PEPROCESS EprocessOfVictim = PsGetCurrentProcess();
		PUNICODE_STRING VictimProcessName = GetProcessNameByEprocess(EprocessOfVictim);

		KdPrint(("%wZ kill %wZ.\r"), CriminalProcessName, VictimProcessName);

		status = EventCheck(CriminalProcessName, VictimProcessName, EprocessOfCriminal, EprocessOfVictim, HookObject->CrimeType, 0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		//No operation because I don't want to refactoring the local variables
	}

	return status;
}
