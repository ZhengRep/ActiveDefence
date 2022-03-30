#include "HookHelper.h"
#include "GlobalVarible.h"
#include "SystemHelper.h"
//Global variable
volatile long __NumberOfRaisedCpu;
KSPIN_LOCK __SpinLock;
KDPC __Dpcs[0x100];

ULONG FindHookPosition()
{
	ULONG ulVirtualAddressIndex;
	for (ulVirtualAddressIndex = __KiSystemService; ulVirtualAddressIndex < __KiSystemService + (PAGE_SIZE / 4); ulVirtualAddressIndex++)
	{
		if (!MmIsAddressValid((PVOID)ulVirtualAddressIndex))
		{
			break;
		}
		if (RtlCompareMemory((PVOID)ulVirtualAddressIndex, __SignatureData, sizeof(__SignatureData)) == sizeof(__SignatureData))
		{
			return ulVirtualAddressIndex;
		}
	}
	return 0;
}

BOOLEAN Hook(IN ULONG HookPosition, IN ULONG Tampoline)
{
	BOOLEAN IsOk = FALSE;

	//Check HookPosition is writable
	PVOID pReflectAddress;
	PMDL pMdl = MakeAddressWritable(HookPosition, 20, pReflectAddress);
	if (!pMdl)
	{
		return FALSE;
	}

	//Hook
	KAFFINITY affinity = KeQueryActiveProcessors();
	ULONG ulActiveProcessorCount = KeQueryActiveProcessorCount(&affinity);

	UCHAR ucTempBuffer[5] = { 0xe9, 0x00, 0x00, 0x00, 0x00 };
	KIRQL irql;
	if (ulActiveProcessorCount == 1)
	{
		irql = KeRaiseIrqlToDpcLevel();

		memcpy(__RecordPosition, pReflectAddress, 5);
		*(PULONG)(&ucTempBuffer[1]) = Tampoline - (HookPosition + 5);
		memcpy(pReflectAddress, ucTempBuffer, 5);

		KeLowerIrql(irql);
		IsOk = TRUE;
	}
	else
	{
		KeInitializeSpinLock(&__SpinLock);

		int i;
		for (i = 0; i < 0x100; ++i)
		{
			KeInitializeDpc(&__Dpcs[i], (PKDEFERRED_ROUTINE)DpcProcedure, NULL); //DpcProcedure 
		}

		KeAcquireSpinLock(&__SpinLock, &irql);
		
		ULONG ulCurrentCPU = KeGetCurrentProcessorNumber();

		for (i = 0; i < 32; i++)
		{
			if (i != ulCurrentCPU)
			{
				KeSetTargetProcessorDpc(&__Dpcs[i], (CCHAR)i);
				KeSetImportanceDpc(&__Dpcs[i], HighImportance);
				KeInsertQueueDpc(&__Dpcs[i], NULL, NULL);
			}
		}
		
		while (1)
		{
			if (__NumberOfRaisedCpu == ulActiveProcessorCount - 1)
			{
				memcpy(__RecordPosition, pReflectAddress, 5);
				*(PULONG)(&ucTempBuffer[1]) = Tampoline - (HookPosition + 5);
				memcpy(pReflectAddress, ucTempBuffer, 5);

				IsOk = TRUE;
				break;
			}
		}

		KeReleaseSpinLock(&__SpinLock, irql);
	}
	MmUnlockPages(pMdl);
	IoFreeMdl(pMdl);	
	return IsOk;
}

VOID DpcProcedure()
{
	KIRQL irql;

	irql = KeRaiseIrqlToDpcLevel();
	InterlockedIncrement(&__NumberOfRaisedCpu);

	KeAcquireSpinLockAtDpcLevel(&__SpinLock);

	KeReleaseSpinLockFromDpcLevel(&__SpinLock);

	KeLowerIrql(irql);
}

BOOLEAN UnHook()
{
	BOOLEAN IsOk = FALSE;

	//Check HookPosition is writable
	PVOID pReflectAddress;
	PMDL pMdl = MakeAddressWritable(__HookPosition, 20, pReflectAddress);
	if (!pMdl)
	{
		return FALSE;
	}

	//Hook
	KAFFINITY affinity = KeQueryActiveProcessors();
	ULONG ulActiveProcessorCount = KeQueryActiveProcessorCount(&affinity);

	KIRQL irql;
	if (ulActiveProcessorCount == 1)
	{
		irql = KeRaiseIrqlToDpcLevel();

		memcpy(pReflectAddress, __RecordPosition, 5);

		KeLowerIrql(irql);
		IsOk = TRUE;
	}
	else
	{
		KeInitializeSpinLock(&__SpinLock);

		int i;
		for (i = 0; i < 0x100; ++i)
		{
			KeInitializeDpc(&__Dpcs[i], (PKDEFERRED_ROUTINE)DpcProcedure, NULL); //DpcProcedure 
		}

		KeAcquireSpinLock(&__SpinLock, &irql);

		ULONG ulCurrentCPU = KeGetCurrentProcessorNumber();

		for (i = 0; i < 32; i++)
		{
			if (i != ulCurrentCPU)
			{
				KeSetTargetProcessorDpc(&__Dpcs[i], (CCHAR)i);
				KeSetImportanceDpc(&__Dpcs[i], HighImportance);
				KeInsertQueueDpc(&__Dpcs[i], NULL, NULL);
			}
		}

		while (1)
		{
			if (__NumberOfRaisedCpu == ulActiveProcessorCount - 1)
			{
				memcpy(pReflectAddress, __RecordPosition, 5);
				IsOk = TRUE;
				break;
			}
		}

		KeReleaseSpinLock(&__SpinLock, irql);
	}
	MmUnlockPages(pMdl);
	IoFreeMdl(pMdl);
	return IsOk;
}

VOID __declspec(naked) FiFastCallEntry()
{
	_asm
	{
		pushad
		pushfd

		push edi
		push edx
		push ebx

		call Fack911
		mov [esp + 0x18], eax //eip??

		popfd
		popad

		sub esp, ecx
		shr ecx, 2
		jmp __JmpBackPosition
	}
}

ULONG __stdcall Fake911(ULONG FunctionService, ULONG FunctionAddress, ULONG ServiceTableBase)
{
	PHOOK_OBJECT HookObject = NULL;

	if (FunctionService >= HOOK_OBJECTS)
		return FunctionAddress;

	if (ServiceTableBase == __ServiceTableBase && FunctionService <= __NumberOfServices)
	{
		HookObject = __FakeSsdt[FunctionService];		// SSDT
	}
	else if (ServiceTableBase == __ShadowServiceTableBase && FunctionService <= __ShadowNumberOfServices)
	{
		HookObject = __FakeSssdt[FunctionService];     // ShadowSSDT
	}

	if (HookObject && HookObject->FakeFunctionAddress)
	{
		if (!(HookObject->FilterType & FILTER_TYPE_KERNEL))
		{

			if (ExGetPreviousMode() == KernelMode)
				return FunctionAddress;
		}
		if (!(HookObject->FilterType & FILTER_TYPE_USER))
		{

			if (ExGetPreviousMode() == UserMode)
				return FunctionAddress;
		}
		// ok
		HookObject->OriginalFunctionAddress = FunctionAddress;
		return HookObject->FakeFunctionAddress;
	}

	return FunctionAddress;
}

BOOLEAN InitializeSysCallTable()
{

}


BOOLEAN IsMajorProtect(IN ULONG CrimeType)
{
	PAGED_CODE();
	return ((CrimeType & CRIME_MAJOR_MASK & __MajorProtectedMask) != 0);
}

BOOLEAN IsAllHookObjectIsUsing()
{
	PAGED_CODE();
	int i;
	for (i = 0; i < HOOK_OBJECTS; i++)
	{
		if ((__FakeSsdt[i] == NULL ) && ((__FakeSsdt[i])->UseReference == 0) || (__FakeSssdt[i] == NULL) && ((__FakeSssdt[i])->UseReference == 0))
		{
			return FALSE;
		}
	}
	return TRUE;
}