# ActiveDefence

## 项目描述

该项目通过驱动和应用相互通信，实现监控进线程并且设置相应的监控信息；通过Hook相应的API实现操作的拦截和过滤。而我在本项目主要完成了NtDeleteFile和NtTerminateProcess两个API，实现对结束进程操作的监控和删除文件的拒绝。

## 项目实现

### 文件目录

```shell
  '    |-- BlackWhiteHashTable.cpp', //添加进线程的黑白名单，每次都会过滤相应的IRP，如果为黑名单中的进程则不可以进行相应操作，白名单中的进程可以进行相应的操作。
  '    |-- BlackWhiteHashTable.h',
  '    |-- Driver.c',
  '    |-- Driver.h',
  '    |-- EventHelper.cpp', //用户的一个操作为被看成一个事件处理
  '    |-- EventHelper.h',
  '    |-- FackSSDT.cpp', //Hook了NtTerminateProcess NtDeleteFile API
  '    |-- FackSSDT.h',
  '    |-- FilterProcedure.cpp', //对于NtTerminateProcess进行判断，如果为保护进程就直接放回，如果是非保护进程则放行操作
  '    |-- FilterProcedure.h',
  '    |-- GlobalVarible.h',
  '    |-- HookHelper.cpp', //此处Hook SSDT
  '    |-- HookHelper.h',
  '    |-- MemoryHelper.cpp', //验证某段内存可读
  '    |-- MemoryHelper.h',
  '    |-- ReadMe.txt',
  '    |-- StringHelper.cpp', //判断文件结尾是否是宽字符
  '    |-- StringHelper.h',
  '    |-- SystemHelper.cpp', //获取SSDT表，申请可写的MDL，获取KiSystemService地址
  '    |-- SystemHelper.h',
```

## 代码逻辑

此处我应该还要重新写一遍。自己设计，自己完成。
