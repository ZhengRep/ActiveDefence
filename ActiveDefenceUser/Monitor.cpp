#include "pch.h"
#include "Monitor.h"
#include <clocale>

HINSTANCE			__Instance = NULL;
HANDLE				__DeviceHandle = INVALID_HANDLE_VALUE;
HANDLE				__ThreadHandle = INVALID_HANDLE_VALUE;
USER_EVENT_DATA		__UserEventData = { 0 };
HANDLE				__EventHandle;
BOOL				__IsLoop = FALSE;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPTSTR lpCmdLine, int nCmdShow)
{
	setlocale(LC_ALL, "ChineseSimplified");
	__Instance = hInstance;
	__DeviceHandle = CreateFile(DRIVER_LINK_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (__DeviceHandle == INVALID_HANDLE_VALUE)
	{
		int LastError = GetLastError();
		TCHAR Message[128] = { 0 };
		_stprintf(Message, _T("(CreateFile() Error = %d)"), LastError);
		MessageBox(NULL, Message, _T("ActiveDefence"), 0);
		return 0;
	}

	__EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	__ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ThreadProcedure, NULL, 0, NULL);
	if (!__ThreadHandle)
	{
		MessageBox(NULL, _T("_beginthreadex() Error"), _T("ActiveDefence"), 0);
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DialogProcedureMain);

	WaitForSingleObject(__ThreadHandle, INFINITE);

	CloseHandle(__ThreadHandle);
	CloseHandle(__DeviceHandle);
	return 0;
}

unsigned WINAPI ThreadProcedureMain(LPVOID ParameterData)
{
	WaitForSingleObject(__EventHandle, INFINITE);
	while (!__IsLoop)
	{
		JUDGMENT_DATA JudgmentData;
		DWORD ReturnLength;

		BOOL IsOk = DeviceIoControl(__DeviceHandle, IOCTL_GET_EVENT_DATA, NULL, 0, &__UserEventData, sizeof(USER_EVENT_DATA), &ReturnLength, NULL);
		if (!IsOk)
		{
			continue;
		}

		DeviceNameToDosName(__UserEventData.Criminal);
		DeviceNameToDosName(__UserEventData.Victim);
		JudgmentData.EventData = __UserEventData.EventData;
		JudgmentData.Judgment = (ULONG)DialogBox(__Instance, MAKEINTRESOURCE(IDD_DIALOG_POP), NULL, DialogProcedureMonitor());

		DeviceIoControl(__DeviceHandle, IOCTL_GIVE_JUDGMENT_DATA, &JudgmentData, sizeof(JUDGMENT_DATA), NULL, 0, &ReturnLength, NULL);
	}
	return 0;
}

INT_PTR WINAPI DialogProcedureMain(HWND Hwnd, UINT Message, WPARAM ParameterData1, LPARAM ParameterData2)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		if (IsMonitorOn(CRIME_MAJOR_PROCESS))
		{
			CheckDlgButton(Hwnd, IDC_START_PROCESS_MONITOR, BST_CHECKED);
		}
		if (IsMonitorOn(CRIME_MAJOR_FILE))
		{
			CheckDlgButton(Hwnd, IDC_START_FILE_MONITOR, BST_CHECKED);
		}
		SetEvent(__EventHandle);
		break;
	}
	case WM_COMMAND:
	{
		short Choice = LOWORD(ParameterData1);
		switch (Choice)
		{
		case IDC_START_PROCESS_MONITOR:
		{
			if (IsDlgButtonChecked(Hwnd, Choice) == BST_CHECKED)
			{
				SetMonitor(CRIME_MAJOR_PROCESS, TRUE);  //通知开启监控
			}
			else if (IsDlgButtonChecked(Hwnd, Choice) == BST_UNCHECKED) //点击消除
			{
				SetMonitor(CRIME_MAJOR_PROCESS, FALSE); //通知关闭监控
			}
			break;
		}
		case IDC_START_FILE_MONITOR:
		{
			if (IsDlgButtonChecked(Hwnd, Choice) == BST_CHECKED)
			{
				SetMonitor(CRIME_MAJOR_FILE, TRUE);
			}
			else if (IsDlgButtonChecked(Hwnd, Choice) == BST_UNCHECKED)
			{
				SetMonitor(CRIME_MAJOR_FILE, FALSE);
			}
			break;
		}
		case WM_CLOSE:
		{
			TerminateThread(__ThreadHandle, 0);
			Sleep(0);
			EndDialog(Hwnd, FALSE);
			break;
		}
		default:
		{
			break;
		}
	}
	return FALSE;
}

BOOL IsMonitorOn(ULONG CrimeType)
{
	MAJOR_INFORMATION MajorInfo;
	MajorInfo.CrimeType = CrimeType;
	MajorInfo.IsProtected = (ULONG)FALSE;

	DWORD ReturnLength;

	BOOL IsOk = DeviceIoControl(__DeviceHandle, IOCTL_GET_MAJOR_INFORMATION, &MajorInfo, sizeof(_MAJOR_INFORMATION_), &MajorInfo, sizeof(_MAJOR_INFORMATION_),&ReturnLength, NULL);

	if (IsOk)
	{
		return (ULONG)MajorInfo.IsProtected;
	}
	else
	{
		return FALSE;
	}
}

BOOL SetMonitor(ULONG CrimeType, BOOL IsOn)
{

	PMAJOR_INFORMATION MajorInfo = (PMAJOR_INFORMATION)VirtualAlloc(NULL, sizeof(MAJOR_INFORMATION), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!MajorInfo)
	{
		return 0;
	}

	MajorInfo->CrimeType = CrimeType;
	MajorInfo->IsProtected = (ULONG)IsOn;

	HANDLE ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)SeSetMonitorInternal, MajorInfo, 0, NULL);

	if (!ThreadHandle)
	{
		return FALSE;
	}

	CloseHandle(ThreadHandle);
	return TRUE;
}

DWORD WINAPI SetMonitorInternal(PMAJOR_INFORMATION MajorInfo)
{
	DWORD	ReturnLength;

	DeviceIoControl(__DeviceHandle, IOCTL_SET_MAJOR_INFORMATION, MajorInfo, sizeof(MAJOR_INFORMATION), NULL, 0, &ReturnLength, NULL);

	VirtualFree(MajorInfo, 0, MEM_RESERVE);
	return ReturnLength;
}

VOID SeDeviceNameToDosName(wchar_t* ImageName)
{
	if (!ImageName)
	{
		return;
	}

	wchar_t TempWStr[512] = L"";

	if (GetLogicalDriveStringsW(512 - 1, TempWStr))
	{
		wchar_t ResultWStr[MAX_PATH];
		wchar_t ImediateWStr[3] = L" :";
		BOOL IsFound = FALSE;
		TCHAR* p = TempWStr;
		do
		{
			// Copy the drive letter to the template string
			*ImediatWStr = *p;
			// Look up each device name
			if (QueryDosDeviceW(ImediatWStr, ResultWStr, 512))
			{
				ULONG Length = (ULONG)wcslen(ResultWStr);
				if (wcslen(ResultWStr) < MAX_PATH)
				{
					IsFound = (_wcsnicmp(ImageName, ResultWStr, Length) == 0);
					if (IsFound)
					{
						// Reconstruct pszFilename using szTempFile
						// Replace device path with DOS path
						wchar_t v5[MAX_PATH];
						_stprintf_s(v5, MAX_PATH, L"%s%s", ImediatWStr, ImageName + Length);
						wcscpy_s(ImageName, MAX_PATH, v5);
					}
				}
			}
			while (*p++);
		} while (!IsFound && *p); // end of string
	}
}