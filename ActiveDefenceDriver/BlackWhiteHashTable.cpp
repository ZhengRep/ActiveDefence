#include "StringHelper.h"
#include "BlackWhiteHashTable.h"

KSPIN_LOCK		__SpinLockOfBlackWhiteHashTable;
PNODE			__BlackWhiteHashTable[0x100] = { 0 };

BOOLEAN InitializeBlackWhiteHashTable()
{
	BOOLEAN IsOk = TRUE;
	KeInitializeSpinLock(&__SpinLockOfBlackWhiteHashTable);

	UNICODE_STRING TempString;
	RtlInitUnicodeString(&TempString, L"csrss.exe");
	IsOk = (IsOk && AddNodeToBlackWhiteHashTable(&TempString, CRIME_MAJOR_ALL, NODE_TYPE_WHITE));

	RtlInitUnicodeString(&TempString, L".cpp");
	IsOk = (IsOk && AddNodeToBlackWhiteHashTable(&TempString, CRIME_MAJOR_FILE, NODE_TYPE_BLACK));

	RtlInitUnicodeString(&TempString, L".c");
	IsOk = (IsOk && AddNodeToBlackWhiteHashTable(&TempString, CRIME_MAJOR_FILE, NODE_TYPE_BLACK));
	
	return IsOk;
}

BOOLEAN AddNodeToBlackWhiteHashTable(PUNICODE_STRING NodeName, ULONG CrimeType, UCHAR NodeType)
{
    PAGED_CODE();
	BOOLEAN IsOk = TRUE;
	//Check parameters
	if (!NodeName)
	{
		return FALSE;
	}
	//Init node
	PNODE pNode = (PNODE)ExAllocatePool(NonPagedPool, sizeof(NODE));
	if (!pNode)
	{
		return FALSE;
	}
	pNode->CrimeType = CrimeType;
	pNode->NodeType = NodeType;
	if (NodeName->Length <= 32)
	{
		RtlCopyMemory(pNode->NodeName, NodeName->Buffer, NodeName->Length);
	}
	else
	{
		RtlCopyMemory(pNode->NodeName, ((PUCHAR)NodeName->Buffer + (NodeName->Length - 32)), sizeof(WCHAR) * 32);
	}
	//Updata table
	KIRQL irql;
	UCHAR Hash = GET_HASH(CrimeType);
	KeAcquireSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);
	pNode->Flink = __BlackWhiteHashTable[Hash];
	__BlackWhiteHashTable[Hash] = pNode;
	KeReleaseSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);

	return IsOk;
}

BOOLEAN IsInBlackWhiteHashTable(PUNICODE_STRING NodeName, ULONG CrimeType, UCHAR NodeType)
{
	PAGED_CODE();
	BOOLEAN IsFound = FALSE;
	KIRQL irql;
	UCHAR Hash = GET_HASH(CrimeType);
	PNODE pNode = __BlackWhiteHashTable[Hash];

	KeAcquireSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);

	while (pNode)
	{
		if (IsUnistrEndWithWcs(NodeName, pNode->NodeName) && (NodeType == pNode->NodeType))
		{

#ifdef DBG
			if (CrimeType != CRIME_MAJOR_ALL)
			{
				if (NodeType == NODE_TYPE_WHITE)
					KdPrintEx((DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "%wZ in White List!\r\n", NodeName));
				else
					KdPrintEx((DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "%wZ in Black List!\r\n", NodeName));
			}
#endif

			IsFound = TRUE;
			break;
		}
	}

	KeReleaseSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);
	return IsFound;
}

VOID EraseBlackWhiteHashTable()
{
	KIRQL irql;
	int i;
	PNODE pNode = NULL;

	KeAcquireSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);

	for (i = 0; i < 0x100; i++)
	{
		pNode = __BlackWhiteHashTable[i];
		__BlackWhiteHashTable[i] = NULL;
		while (pNode->Flink)
		{
			PNODE pTempNode = pNode;
			pNode = pNode->Flink;
			ExFreePool(pTempNode);
		}
	}

	KeReleaseSpinLock(&__SpinLockOfBlackWhiteHashTable, &irql);
}
