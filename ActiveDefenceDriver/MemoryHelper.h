#pragma once

#include<fltKernel.h>

#define MEMORY_USER_MODE	0X4
#define MEMORY_KERNEL_MODE	0X8

VOID ProbeForReadOwn(IN PVOID  VirtualAddress, IN ULONG ViewSize, IN ULONG Alignment, IN  ULONG MemoryType);
