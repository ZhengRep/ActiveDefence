#pragma once
#include<fltKernel.h>

#define MAX_PATH				256


extern NPAGED_LOOKASIDE_LIST __EventDataNpagedLookasideList;

typedef struct _JUDGMENT_DATA {
	ULONG				EventData;
	ULONG				Judgment;
} JUDGMENT_DATA, * PJUDGMENT_DATA, ** PPJUDGMENT_DATA;

typedef struct _EVENT_DATA_ {
	LIST_ENTRY			ListEntry;
	KEVENT				KEvent;
	ULONG				CrimeType;
	ULONG				Judgment;
	ULONG				ExtraData;
	PEPROCESS			CriminalProcess;
	PEPROCESS			VictimProcess;
	UNICODE_STRING		Criminal;
	UNICODE_STRING		Victim;
	WCHAR				CriminalBuffer[MAX_PATH];
	WCHAR				VictimBuffer[MAX_PATH];
} EVENT_DATA, * PEVENT_DATA, ** PPEVENT_DATA;

typedef struct _USER_EVENT_DATA {
	ULONG				EventData;
	ULONG				CrimeType;
	ULONG				ExtraData;
	WCHAR				Criminal[262];
	WCHAR				Victim[262];
} USER_EVENT_DATA, * PUSER_EVENT_DATA, ** PPUSER_EVENT_DATA;

BOOLEAN InitializeEventHandler();
VOID SetMajorProtectType(IN ULONG CrimeType, IN BOOLEAN IsOn);
NTSTATUS EventCheck(
	IN PUNICODE_STRING		 Criminal,
	IN PUNICODE_STRING		 Victim,
	IN PEPROCESS			CriminalProcess,
	IN PEPROCESS			VictimProcess,
	IN ULONG				CrimeType,
	IN ULONG				ExtraData
);

PEVENT_DATA
BuildEventData(
	IN PUNICODE_STRING		Criminal,
	IN PUNICODE_STRING		Victim,
	IN PEPROCESS			CriminalProcess,
	IN PEPROCESS			VictimProcess,
	IN ULONG				CrimeType,
	IN ULONG				ExtraData
);
VOID BuildUserEventData(IN PEVENT_DATA EventData, OUT PUSER_EVENT_DATA UserEvevtData);
VOID PushEventData(IN PEVENT_DATA EventData);
VOID DestroyEventData(IN PEVENT_DATA EventData);
VOID CancelAllEventData();