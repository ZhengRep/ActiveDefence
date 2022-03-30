#include "Driver.h"
#include "SystemHelper.h"
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


	return status;
}

NTSTATUS IoCloseDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	return NTSTATUS();
}

NTSTATUS IoDeviceControlDispatch(IN PDEVICE_OBJECT DevObj, IN PIRP irp)
{
	return NTSTATUS();
}

BOOLEAN DriverEntryInit()
{
	BOOLEAN IsOk = FALSE;

	return IsOk;
}

