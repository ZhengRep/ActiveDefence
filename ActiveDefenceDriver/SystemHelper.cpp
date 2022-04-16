#include "SystemHelper.h"
#include "GlobalVarible.h"
/*
TODO:

*/

BOOLEAN InitCommonVariables()
{
	PAGED_CODE();
	//high user address
	UNICODE_STRING TempString;
	RtlInitUnicodeString(&TempString, L"MmUserProbeAddress");
	__MmUserProbeAddress = (ULONG)MmGetSystemRoutineAddress(&TempString);
	//get service
	RtlInitUnicodeString(&TempString, L"KeServiceDiscriptorTable");
	__KeServiceDescriptorTable = (ULONG)MmGetSystemRoutineAddress(&TempString); //获取SSDT的基址
	if (__KeServiceDescriptorTable && MmIsAddressValid((PVOID)__KeServiceDescriptorTable))
	{
		__ServiceTableBase = *(PULONG)__KeServiceDescriptorTable;
		__NumberOfServices = *(PULONG)(__KeServiceDescriptorTable + 8);
	}
	//Hook
	__HookPosition = FindHookPosition();
	__JmpBackPosition = (__HookPosition)+5;
	//SystemService
	__KiSystemService = GetKiSystemAddress();
	//ShadowSsdt
	GetShadowSsdtInfo(&__ShadowServiceTableBase, &__ShadowNumberOfServices);

	//Check
	if (!__MmUserProbeAddress || !MmIsAddressValid((PVOID)__MmUserProbeAddress) ||
		!__ServiceTableBase || !MmIsAddressValid((PVOID)__ServiceTableBase) ||
		!__HookPosition || !MmIsAddressValid((PVOID)__HookPosition) ||
		!__JmpBackPosition || !MmIsAddressValid((PVOID)__JmpBackPosition) ||
		!__KiSystemService || !MmIsAddressValid((PVOID)__KiSystemService) ||
		!__ShadowServiceTableBase || !MmIsAddressValid((PVOID)__ShadowServiceTableBase) ||
		!__NumberOfServices || !__ShadowNumberOfServices)
	{
		return FALSE;
	}
	return TRUE;
}

BOOLEAN GetShadowSsdtInfo(IN PULONG ServiceTableBase, IN PULONG ServiceNumber)
{
	PAGED_CODE();
	BOOLEAN IsOk = FALSE;
	UNICODE_STRING TempString = RTL_CONSTANT_STRING(L"KeAddSystemServiceTable");
	ULONG KeAddSystemServiceTable = (ULONG)MmGetSystemRoutineAddress(&TempString);
	if (!KeAddSystemServiceTable || !MmIsAddressValid((PVOID)KeAddSystemServiceTable)) //KeAddSystemServiceTable ---> SSDTShadow
	{
		return FALSE;
	}
	ULONG TableLimitAddress = KeAddSystemServiceTable + 256;
	while (KeAddSystemServiceTable < TableLimitAddress)
	{
		if (MmIsAddressValid((PVOID)KeAddSystemServiceTable) && *(USHORT*)KeAddSystemServiceTable == 0x888d) //搜索特征码0x888d 来寻找
		{
			ULONG KeServiceDiscriptorShadowTable = *(PULONG)(KeAddSystemServiceTable + 2);
			*ServiceTableBase = *(PULONG)KeServiceDiscriptorShadowTable + 16;
			*ServiceNumber = *(PULONG)KeServiceDiscriptorShadowTable + 24;
		}
		KeAddSystemServiceTable++;
	}
	return IsOk;
}

//枚举系统模块
BOOLEAN GetSystemModuleByName(IN char* ModuleName, OUT PSYSTEM_MODULE_ENTRY SystemModuleEntry)
{
	PAGED_CODE();
	BOOLEAN IsOk;
	PSYSTEM_MODULE_INFORMATION SystemModuleInfo = GetSystemModule();
	if (!SystemModuleInfo)
	{
		return FALSE;
	}
	ULONG NumberOfModules = SystemModuleInfo->NumberOfModules;
	PSYSTEM_MODULE_ENTRY SystemModuleEntrys = SystemModuleInfo->Modules;
	int i;
	for (i = 0; i < NumberOfModules; i++)
	{
		if (!SystemModuleEntrys)
		{
			char* TempStr = strrchr((const char*)SystemModuleEntrys->ModuleName, '\\');
			if (TempStr)
			{
				TempStr++;
			}
			else
			{
				TempStr = (char*)SystemModuleEntrys->ModuleName;
			}
			if (!stricmp(TempStr, ModuleName))
			{
				RtlCopyMemory(SystemModuleEntry, TempStr, sizeof(SYSTEM_MODULE_ENTRY));
				IsOk = TRUE;
				break;
			}
		}
		SystemModuleEntrys++;
	}
	return IsOk;
}

PSYSTEM_MODULE_INFORMATION GetSystemModule()
{
	PAGED_CODE();
	ULONG VirtualSize = 512;
	NTSTATUS status;
	PSYSTEM_MODULE_INFORMATION smi;
	while (TRUE)
	{
		smi = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(PagedPool, VirtualSize);
		if (!smi)
		{
			return NULL;
		}
		status = ZwQuerySystemInformation(SystemModuleInformation, smi, VirtualSize, &VirtualSize); //获取ntoskrnl模块的信息
		if (status != STATUS_INFO_LENGTH_MISMATCH)
		{
			break;
		}
		ExFreePool(smi);
	}
	if (!NT_SUCCESS(status))
	{
		ExFreePool(smi);
		return NULL;
	}
	return smi;
}

ULONG GetKiSystemAddress()
{
	IDT_INFORMATION IDTInfo;
	_asm
	{
		__sidt IDTInfo;
	}
	PIDT_ENTRY IDTEntry = (PIDT_ENTRY)((IDTInfo.HighIdtBase << 16) | IDTInfo.LowIdtBase);

	if (IDTEntry)
	{
		ULONG KiSystemService = (ULONG)((IDTEntry[0x2e].HighOffset << 16) | IDTEntry[0x2e].LowOffset);
		if (KiSystemService && MmIsAddressValid((PVOID)KiSystemService)) {
			return KiSystemService;
		}
	}
	return 0;
}

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

	PUCHAR VirtualAddress;
	USHORT VirtualSize = 32;
	while (TRUE)
	{
		VirtualAddress = (PUCHAR)ExAllocatePool(NonPagedPool, VirtualSize);
		if (!VirtualAddress)
		{
			return NULL;
		}
		status = ZwQueryInformationProcess(EprocessHandle, ProcessImageFileName, VirtualAddress, VirtualSize, (PULONG)VirtualSize);
		if (status != STATUS_INFO_LENGTH_MISMATCH)
		{
			break;
		}
		ExFreePool(VirtualAddress);
	}

	ZwClose(EprocessHandle);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(VirtualAddress);
		return NULL;
	}
	
	return (PUNICODE_STRING)VirtualAddress;
}