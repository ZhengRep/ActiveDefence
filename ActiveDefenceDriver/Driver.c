#include "Driver.h"
#include "SystemHelper.h"
#include "MemoryHelper.h"
#include "EventHelper.h"
#include "HookHelper.h"

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriObj, IN UNICODE_STRING RegPath)
{
	NTSTATUS status;
	UNICODE_STRING DevName;
	RtlInitUnicodeString(&DevName, DEVICE_OBJECT_NAME);

	//Create device
	PDEVICE_OBJECT DevObj;
	status = IoCreateDevice(DriObj, NULL, &DevName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DevObj);
	if (!(NT_SUCCESS(status)))
	{
		return STATUS_UNSUCCESSFUL;
	}

	//Create SymbolLink
	UNICODE_STRING DevSymLinkName;
	RtlInitUnicodeString(&DevSymLinkName, DEVICE_LINK_NAME);
	status = IoCreateSymbolicLink(&DevSymLinkName, &DevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&DevSymLinkName);
		status = IoCreateSymbolicLink(&DevSymLinkName, &DevName);
		if (!NT_SUCCESS(status))
		{
			IoDeleteDevice(DevObj);
			return STATUS_UNSUCCESSFUL;
		}
	}

	//Dispatch
	int i;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriObj->MajorFunction[i] = IoPassThroughDispatch;
	}
	DriObj->MajorFunction[IRP_MJ_CREATE] = IoCreateDispatch;
	DriObj->MajorFunction[IRP_MJ_CLOSE] = IoCloseDispatch;						//CloseHandle(ThreadHandle)   
	DriObj->MajorFunction[IRP_MJ_CLEANUP] = IoCloseDispatch;					//CloseHanlde(FileHandle)
	DriObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoDeviceControlDispatch;
	DriObj->DriverUnload = DriverUnload;

	if (!DriverEntryInit())
	{
		return STATUS_UNSUCCESSFUL;
	}

	__Ready = TRUE;
	return status;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriObj)
{
	PDEVICE_OBJECT DevObj = DriObj;
	PDEVICE_OBJECT TempDevObj;
	while (DevObj)
	{
		TempDevObj = DevObj;
		DevObj = DevObj->NextDevice;
		IoDeleteDevice(TempDevObj);
	}

	UnHook();
	CancelAllEventData();
	KeDelayExecutionThread(KernelMode, FALSE, LOW_REALTIME_PRIORITY);
	if (IsAllHookObjectIsUsing())
	{
		LARGE_INTEGER interval;
		interval.QuadPart = -2LL * 1000LL * 1000LL * 10LL;
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}
	ExDeleteNPagedLookasideList(&__EventDataNpagedLookasideList);
	EraseBlackWhiteHashTable();
}

NTSTATUS IoPassThroughDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS IoCreateDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	NTSTATUS status;

	if (__Ready)
	{
		//Get EProcess
		PEPROCESS EProcess = PsGetCurrentProcess();
		if (EProcess)
		{
			PUNICODE_STRING ImageName = GetProcessNameByEprocess(EProcess);
			if (ImageName)
			{
				AddNodeToBlackWhiteHashTable(ImageName, CRIME_MAJOR_ALL, NODE_TYPE_WHITE); //将自己的进程加入白名单
				ExFreePool(ImageName);
				CancelAllEventData();
				__OwnProcess = EProcess;
				KdPrint(("OwnProcess is %p", __OwnProcess));
				status = STATUS_SUCCESS;
			}
		}
	}
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS IoCloseDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	PAGED_CODE();
	CancelAllEventData();
	__OwnProcess = NULL;
	KdPrint(("Own process is close.\n"));

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	return STATUS_SUCCESS;
}

NTSTATUS IoDeviceControlDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
	ULONG InputBufferLength = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutputBufferLength = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG IoControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	switch (IoControlCode)
	{
	case IOCTL_GET_EVENT_DATA: //应用层获取事件数据 接受操作
	{
		if (InputBufferLength == sizeof(USER_EVENT_DATA))
		{
			LARGE_INTEGER TimeOut;
			TimeOut.QuadPart = -3LL * 1000LL * 1000LL * 10LL;
			PEVENT_DATA EventData = PopEventData(&TimeOut);
			BuildUserEventData(EventData, irp->AssociatedIrp.SystemBuffer);
			irp->IoStatus.Information = sizeof(USER_EVENT_DATA);
			irp->IoStatus.Status = STATUS_SUCCESS;
		}
		break;
	}
	case IOCTL_GET_MAJOR_INFORMATION:  //查看监控信息
	{
		if (InputBufferLength == sizeof(MAJOR_INFORMATION) && OutputBufferLength == sizeof(MAJOR_INFORMATION))
		{
			PMAJOR_INFORMATION MajorInfo = irp->AssociatedIrp.SystemBuffer;
			__try
			{
				ProbeForReadOwn(MajorInfo, sizeof(MAJOR_INFORMATION), 1, MEMOERY_KERNEL_MODE);
				MajorInfo->IsProtected = IsMajorProtect(MajorInfo->CrimeType);
				irp->IoStatus.Information = sizeof(MAJOR_INFORMATION);
				irp->IoStatus.Status = STATUS_SUCCESS;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{

			}
		}
		break;
	}
	case IOCTL_GIVE_JUDGMENT_DATA: //设置监控信息
	{
		if (InputBufferLength == sizeof(JUDGMENT_DATA)) {
			PJUDGMENT_DATA JudgmentData = irp->AssociatedIrp.SystemBuffer;
			PEVENT_DATA EventData = JudgmentData->EventData;
			__try
			{
				ProbeForRead(EventData, sizeof(EVENT_DATA), 1, MEMOERY_KERNEL_MODE);
				EventData->Judgment = JudgmentData->Judgment;
				KeSetEvent(&EventData->KEvent, IO_NO_INCREMENT, FALSE);
				irp->IoStatus.Status = STATUS_SUCCESS;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{

			}
		}
		break;
	}
	case IOCTL_SET_MAJOR_INFORMATION: //拒绝操作状态
	{
		if (InputBufferLength == sizeof(MAJOR_INFORMATION))
		{
			PMAJOR_INFORMATION MajorInfo = irp->AssociatedIrp.SystemBuffer;
			__try
			{
				ProbeForReadOwn(MajorInfo, sizeof(MAJOR_INFORMATION), 1, MEMOERY_KERNEL_MODE);
				SetMajorProtectType(MajorInfo->CrimeType, MajorInfo->IsProtected);
				MajorInfo->IsProtected = IsMajorProtect(MajorInfo->CrimeType);
				irp->IoStatus.Status = STATUS_SUCCESS;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{

			}
		}
		break;
	}
	default:
		break;
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

BOOLEAN DriverEntryInit()
{
	BOOLEAN IsOk = FALSE;
	//Event
	if (!InitializeEventHandler())
	{
		return FALSE;
	}
	//Ssdt Sssdt
	if (!InitCommonVariables())
	{
		ExFreeToNPagedLookasideList(&__EventDataNpagedLookasideList, NULL);
		return FALSE;
	}
	//BlackWhiteTable
	if (!InitializeBlackWhiteHashTable())
	{
		ExFreeToNPagedLookasideList(&__EventDataNpagedLookasideList, NULL);
		return FALSE;
	}
	//Hook
	if (!Hook(__HookPosition, (ULONG)FakeKiFastCallEntry))
	{
		ExFreeToNPagedLookasideList(&__EventDataNpagedLookasideList, NULL);
		return FALSE;
	}
	return TRUE;
}

