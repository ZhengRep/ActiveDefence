#pragma once
#include "GlobalVarible.h"
 
#define GET_HASH(CrimeType)	(UCHAR)((((ULONG)(CrimeType)) >> 24) | (((ULONG)(CrimeType)) & 0xF))

typedef struct _NODE {
	struct _NODE*	Flink;
	ULONG			CrimeType;
	UCHAR			NodeType;
	WCHAR			NodeName[32];
}NODE, * PNODE, ** PPNODE;

BOOLEAN InitializeBlackWhiteHashTable();
BOOLEAN AddNodeToBlackWhiteHashTable(PUNICODE_STRING NodeName, ULONG CrimeType, UCHAR NodeType);
BOOLEAN IsInBlackWhiteHashTable(PUNICODE_STRING NodeName, ULONG CrimeType, UCHAR NodeType);
VOID EraseBlackWhiteHashTable();

