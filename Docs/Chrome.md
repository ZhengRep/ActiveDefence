# Chrome

- KeAddSystemServiceTable

This function allows the caller add a system service table to the system.

```c++
BOOLEAN
KeAddSystemServiceTable(
  IN  PULONG Base,
  IN  PULONG Count OPTIONAL,
  IN  ULONG Limit,
  IN  PUCHAR Number,
  IN  ULONG Index
  );
```

寻找KeSystemServiceDispatchTable和KeSystemServiceDispatchShadow在同一页。

- How to get the address of SSDT

```c++
typedef struct tag_SYSTEM_SERVICE_TABLE {
    PNTPROC   ServiceTable; // array of entry points to the calls
    int  CounterTable; // array of usage counters
    ULONG ServiceLimit; // number of table entries
    PCHAR ArgumentTable; // array of argument counts
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE, **PPSYSTEM_SERVICE_TABLE;
```

- ZwQuerySystemInformation

```c++
NTSTATUS WINAPI ZwQuerySystemInformation(
  _In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
  _Inout_   PVOID                    SystemInformation,
  _In_      ULONG                    SystemInformationLength,
  _Out_opt_ PULONG                   ReturnLength
);
```

**SystemInformationClass**

| Value                        | Description                                                  |
| ---------------------------- | ------------------------------------------------------------ |
| SystemBasicInformation       | The number of processors in the system *GetSystemInfo GetNativeSystemInfo* |
| SystemPerformanceInformation | can be used to generate an unpredictable seed for a random number generato |
| SystemTimeOfDayInformation   |                                                              |
| SystemProcessInformation     | An array of SYSTEM_PROCESS_INFORMATION structures, one for each process running in the system. |
| SystemExceptationInformation |                                                              |
| SystemLookasideInformation   |                                                              |

**SystemInformation**

A pointer to a buffer that receives the requested information.

```c++
typedef struct _SYSTEM_BASIC_INFORMATION {
    BYTE Reserved1[24];
    PVOID Reserved2[4];
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset; //NextEntry
    ULONG NumberOfThreads;  //
    BYTE Reserved1[48];
    PVOID Reserved2[3];
    HANDLE UniqueProcessId;
    PVOID Reserved3;
    ULONG HandleCount; //句柄数量 GetProcessHandleC
    BYTE Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage; //
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION;
```



| Value | Description |
| ----- | ----------- |
|       |             |

















****

## Reference:

1. [How get address of SSDT Shadow on Windows10](https://social.msdn.microsoft.com/Forums/en-US/50512d03-fd55-4a66-899f-f33cc96d0551/how-get-address-of-ssdt-shadow-on-windows-10-x32?forum=wdk)
2. 

