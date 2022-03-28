#include "MemoryHelper.h"
/*
TODO:
MmSystemRangeStart:

*/
VOID ProbeForReadOwn(IN PVOID VirtualAddress, IN ULONG ViewSize, IN ULONG Alignment, IN  ULONG MemoryType)
{
	PAGED_CODE();
	
	if (VirtualAddress >= MmSystemRangeStart)
	{
		//Kernel Mode
		if (!(MemoryType & MEMORY_KERNEL_MODE))
		{
			ExRaiseAccessViolation();
			return;
		}

		//VirtualAddress is aligned
		if (!((ULONG)VirtualAddress & (Alignment - 1)))
		{
			ExRaiseDatatypeMisalignment();
			return;
		}

		//Scan block
		ULONG StartAddress = ((ULONG)VirtualAddress / PAGE_SIZE) * PAGE_SIZE;
		ULONG EndAddress = ((ULONG)VirtualAddress + ViewSize) / PAGE_SIZE * PAGE_SIZE;
		while (StartAddress < EndAddress)
		{
			if (!MmIsAddressValid((PVOID)StartAddress))
			{
				ExRaiseAccessViolation();
				return;
			}
			StartAddress += Alignment;
		}
	}
	else
	{
		//User mode
		if (!(MemoryType & MEMORY_USER_MODE))
		{
			ExRaiseAccessViolation();
			return;
		}
		ProbeForRead(VirtualAddress, ViewSize, Alignment);
	}	
}