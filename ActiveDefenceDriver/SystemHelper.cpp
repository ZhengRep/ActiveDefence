#include "SystemHelper.h"
#include "GlobalVarible.h"
/*
TODO:

*/


PMDL MakeAddressWritable(IN ULONG VirtuallAddress, IN ULONG AddressSize, OUT PVOID ReflectAddress)
{
	PAGED_CODE();

	PMDL pMdl = IoAllocateMdl((PVOID)VirtuallAddress, AddressSize, FALSE, TRUE, NULL);
	if (!pMdl)
	{
		return NULL;
	}

	__try
	{
		MmProbeAndLockPages(pMdl, KernelMode, IoWriteAccess);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		IoFreeMdl(pMdl);
		return NULL;
	}

	if (pMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL))
	{
		ReflectAddress = pMdl->MappedSystemVa;
	}
	else
	{
		ReflectAddress = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
	}

	if (!ReflectAddress)
	{
		MmUnlockPages(pMdl);
		IoFreeMdl(pMdl);
		return NULL;
	}

	return pMdl;

}

PUNICODE_STRING GetProcessNameByEprocess(IN PEPROCESS Eprocess)
{
	PAGED_CODE();
	NTSTATUS status;
	HANDLE EprocessHandle;
	status = ObOpenObjectByPointer(Eprocess, NULL, NULL, 0, 0, KernelMode, &EprocessHandle);
	if (!NT_SUCCESS(status))
	{
		return NULL;
	}

	USHORT VirtualSize = 32;
	PUCHAR VirtualAddress = (PUCHAR)ExAllocatePool(NonPagedPool, VirtualSize);
	if (!VirtualAddress)
	{
		return NULL;
	}
	status = ZwQueryInformationProcess()

}