#include "EventHelper.h"
#include "HookHelper.h"
#include "BlackWhiteHashTable.h"
#include "GlobalVarible.h"

extern NPAGED_LOOKASIDE_LIST __EventDataNpagedLookasideList;

KSPIN_LOCK		__SpinLockEventData;
KSEMAPHORE		__SemaphoreEventData;
ULONG			__EventDataCount;

BOOLEAN InitializeEventHandler()
{
	PAGED_CODE();

	BOOLEAN IsOk = FALSE;
	//Init
	KeInitializeSpinLock(&__SpinLockEventData);
	KeInitializeSemaphore(&__SemaphoreEventData, 0, MAXLONG);
	InitializeListHead(&__EventDataListHead);
	ExInitializeNPagedLookasideList(&__EventDataNpagedLookasideList, NULL, NULL, 0, sizeof(EVENT_DATA), ALLOCATE_TAG, 0);
	__EventDataCount = 0;
	__MajorProtectedMask = 0; //No protect process
	
	//Open monitoring
	SetMajorProtectType(CRIME_MAJOR_ALL, TRUE);
	
	return IsOk;
}

VOID SetMajorProtectType(IN ULONG CrimeType, IN BOOLEAN IsOn)
{
	PAGED_CODE();

	if (IsOn)
	{
		KdPrint(("CrimeType %x\n", CrimeType));
		__MajorProtectedMask |= (CrimeType & CRIME_MAJOR_MASK);
		KdPrint(("ON   %x\n", __MajorProtectedMask));
	}
	else
	{
		KdPrint(("CrimeType %x\n", CrimeType));
		__MajorProtectedMask &= ~(CrimeType & CRIME_MAJOR_MASK);
		KdPrint(("OFF   %x\n", __MajorProtectedMask));
	}
}

NTSTATUS EventCheck(
	IN PUNICODE_STRING		 Criminal,
	IN PUNICODE_STRING		 Victim,
	IN PEPROCESS			CriminalProcess,
	IN PEPROCESS			VictimProcess,
	IN ULONG				CrimeType,
	IN ULONG				ExtraData
)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	//Ring3 is not working
	if (!__OwnProcess)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	//sys is not monitoring
	if (!IsMajorProtect(CrimeType))
	{
		return STATUS_UNSUCCESSFUL;
	}

	//is csrss.exe
	if (IsInBlackWhiteHashTable(Criminal, CRIME_MAJOR_ALL, NODE_TYPE_WHITE))
	{
		return STATUS_SUCCESS;
	}
	if (IsInBlackWhiteHashTable(Criminal, CrimeType, NODE_TYPE_WHITE))
	{
		return STATUS_SUCCESS;
	}
	if (IsInBlackWhiteHashTable(Criminal, CrimeType, NODE_TYPE_BLACK))
	{
		return STATUS_UNSUCCESSFUL;
	}

	switch (CrimeType & CRIME_MAJOR_MASK)
	{
	case CRIME_MAJOR_FILE:
		break;
	case CRIME_MAJOR_PROCESS:
		if (VictimProcess == __OwnProcess)
			return STATUS_UNSUCCESSFUL;
		break;
	case CRIME_MAJOR_REGISTRY:
		break;
	case CRIME_MAJOR_SYS:
		break;
	}

	PEVENT_DATA EventData = BuildEventData(Criminal, Victim, CriminalProcess, VictimProcess, CrimeType, ExtraData);
	if (!EventData)
	{
		return STATUS_UNSUCCESSFUL;
	}

	PushEventData(EventData);

	return status;
}

PEVENT_DATA
BuildEventData(
	IN PUNICODE_STRING		Criminal,
	IN PUNICODE_STRING		Victim,
	IN PEPROCESS			CriminalProcess,
	IN PEPROCESS			VictimProcess,
	IN ULONG				CrimeType,
	IN ULONG				ExtraData
)
{
	PAGED_CODE();
	PEVENT_DATA EventData = (PEVENT_DATA)ExAllocateFromNPagedLookasideList(&__EventDataNpagedLookasideList);
	EventData->CriminalProcess = CriminalProcess;
	EventData->VictimProcess = VictimProcess;

	EventData->Criminal.Length = EventData->Victim.Length = 0;
	EventData->Criminal.MaximumLength = EventData->Victim.MaximumLength = MAX_PATH;
	EventData->Criminal.Buffer = EventData->CriminalBuffer;
	EventData->Victim.Buffer = EventData->VictimBuffer;

	if (Victim)
	{
		RtlCopyUnicodeString(&EventData->Criminal, Criminal);
	}
	else
	{
		wcscpy(EventData->Criminal.Buffer, L"UNKNOWN");
		EventData->Criminal.Length = 14;
	}
	if (Victim)
	{
		RtlCopyUnicodeString(&EventData->Victim, Victim);
	}
	else if(CrimeType & CRIME_MAJOR_ALL)
	{
		wcscpy(EventData->Victim.Buffer, L"System-wide");
		EventData->Victim.Length = 22;
	}
	else
	{
		wcscpy(EventData->Victim.Buffer, L"UNKNOWN");
		EventData->Victim.Length = 14;
	}

	return EventData;
}

VOID BuildUserEventData(IN PEVENT_DATA EventData, OUT PUSER_EVENT_DATA UserEvevtData)
{
	PAGED_CODE();
	memset(UserEvevtData, 0, sizeof(USER_EVENT_DATA));
	UserEvevtData->EventData = (ULONG)EventData;
	UserEvevtData->CrimeType = EventData->CrimeType;
	UserEvevtData->ExtraData = EventData->ExtraData;
	RtlCopyMemory(UserEvevtData->Criminal, EventData->CriminalBuffer, EventData->Criminal.Length);
	RtlCopyMemory(UserEvevtData->Victim, EventData->VictimBuffer, EventData->Victim.Length);
}

VOID PushEventData(IN PEVENT_DATA EventData)
{
	PAGED_CODE();
	ExInterlockedInsertTailList(&__EventDataListHead, &EventData->ListEntry, &__SpinLockEventData);
	InterlockedIncrement((volatile long*)&__EventDataCount);
	KeReleaseSemaphore(&__SemaphoreEventData, 0, 1, FALSE);
}

VOID DestroyEventData(IN PEVENT_DATA EventData)
{
	PAGED_CODE();
	ExFreeToNPagedLookasideList(&__EventDataNpagedLookasideList, EventData);
}

VOID CancelAllEventData()
{
	PAGED_CODE();
	InterlockedExchange((volatile long*)&__EventDataCount, 50 + 1);

	while (KeReleaseSemaphore(&__SemaphoreEventData, 0, 1, NULL))
	{
		KeWaitForSingleObject(&__SemaphoreEventData, Executive, KernelMode, FALSE, NULL);
		PLIST_ENTRY TempListEntry = ExInterlockedRemoveHeadList(&__EventDataListHead, &__SpinLockEventData);
		PEVENT_DATA EventData = CONTAINING_RECORD(TempListEntry, EVENT_DATA, ListEntry);
		EventData->Judgment = JUDGMENT_REFUSE;
		KeSetEvent(&EventData->KEvent, IO_NO_INCREMENT, FALSE);
	}

	InterlockedExchange((volatile long*)&__EventDataCount, 0);

}
PEVENT_DATA PopEventData(IN PLARGE_INTEGER TimeOut)
{
	PAGED_CODE();
	NTSTATUS status = KeWaitForSingleObject(&__SemaphoreEventData, Executive, KernelMode, FALSE, TimeOut);
	if (status == STATUS_TIMEOUT || !NT_SUCCESS(status))
	{
		return NULL;
	}
	PLIST_ENTRY ListEntryData = ExInterlockedRemoveHeadList(&__EventDataListHead, &__SpinLockEventData);
	PEVENT_DATA EventData = CONTAINING_RECORD(ListEntryData, EVENT_DATA, ListEntry);
	InterlockedDecrement((volatile long*)__EventDataCount);
	return EventDAta;
}