.macro DEF_SVC name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.align 2
	.cfi_startproc
\name:
.endm

.macro END_DEF_SVC
	.cfi_endproc
.endm

DEF_SVC _ZN3bio3svc11SetHeapSizeERPvy
	str x0, [sp, #-16]!
	svc 0x1
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc19SetMemoryPermissionEPvyj
	svc 0x2
	ret
END_DEF_SVC

DEF_SVC svcSetMemoryAttribute
	svc 0x3
	ret
END_DEF_SVC

DEF_SVC svcMapMemory
	svc 0x4
	ret
END_DEF_SVC

DEF_SVC svcUnmapMemory
	svc 0x5
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11QueryMemoryERNS0_10MemoryInfoERjy
	str x1, [sp, #-16]!
	svc 0x6
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11ExitProcessEv
	svc 0x7
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc12CreateThreadERjPvS2_S2_ii
	str x0, [sp, #-16]!
	svc 0x8
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11StartThreadEj
	svc 0x9
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc10ExitThreadEv
	svc 0xA
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11SleepThreadEx
	svc 0xB
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc17GetThreadPriorityERij
	str x0, [sp, #-16]!
	svc 0xC
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSetThreadPriority
	svc 0xD
	ret
END_DEF_SVC

DEF_SVC svcGetThreadCoreMask
	stp x0, x1, [sp, #-16]!
	svc 0xE
	ldp x3, x4, [sp], #16
	str w1, [x3]
	str x2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcSetThreadCoreMask
	svc 0xF
	ret
END_DEF_SVC

DEF_SVC svcGetCurrentProcessorNumber
	svc 0x10
	ret
END_DEF_SVC

DEF_SVC svcSignalEvent
	svc 0x11
	ret
END_DEF_SVC

DEF_SVC svcClearEvent
	svc 0x12
	ret
END_DEF_SVC

DEF_SVC svcMapSharedMemory
	svc 0x13
	ret
END_DEF_SVC

DEF_SVC svcUnmapSharedMemory
	svc 0x14
	ret
END_DEF_SVC

DEF_SVC svcCreateTransferMemory
	str x0, [sp, #-16]!
	svc 0x15
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11CloseHandleEj
	svc 0x16
	ret
END_DEF_SVC

DEF_SVC svcResetSignal
	svc 0x17
	ret
END_DEF_SVC

DEF_SVC svcWaitSynchronization
	str x0, [sp, #-16]!
	svc 0x18
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcCancelSynchronization
	svc 0x19
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc13ArbitrateLockEjPjj
	svc 0x1A
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc15ArbitrateUnlockEPj
	svc 0x1B
	ret
END_DEF_SVC

DEF_SVC svcWaitProcessWideKeyAtomic
	svc 0x1C
	ret
END_DEF_SVC

DEF_SVC svcSignalProcessWideKey
	svc 0x1D
	ret
END_DEF_SVC

DEF_SVC svcGetSystemTick
	svc 0x1E
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc18ConnectToNamedPortERjPKc
	str x0, [sp, #-16]!
	svc 0x1F
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSendSyncRequestLight
	svc 0x20
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc15SendSyncRequestEj
	svc 0x21
	ret
END_DEF_SVC

DEF_SVC svcSendSyncRequestWithUserBuffer
	svc 0x22
	ret
END_DEF_SVC

DEF_SVC svcSendAsyncRequestWithUserBuffer
	str x0, [sp, #-16]!
	svc 0x23
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc12GetProcessIdERyj
	str x0, [sp, #-16]!
	svc 0x24
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc11GetThreadIdERyj
	str x0, [sp, #-16]!
	svc 0x25
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcBreak
	svc 0x26
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc17OutputDebugStringEPKcy
	svc 0x27
	ret
END_DEF_SVC

DEF_SVC svcReturnFromException
	svc 0x28
	ret
END_DEF_SVC

DEF_SVC _ZN3bio3svc7GetInfoERyjjy
	str x0, [sp, #-16]!
	svc 0x29
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcFlushEntireDataCache
	svc 0x2A
	ret
END_DEF_SVC

DEF_SVC svcFlushDataCache
	svc 0x2B
	ret
END_DEF_SVC

DEF_SVC svcMapPhysicalMemory
	svc 0x2C
	ret
END_DEF_SVC

DEF_SVC svcUnmapPhysicalMemory
	svc 0x2D
	ret
END_DEF_SVC

DEF_SVC svcGetDebugFutureThreadInfo
	stp x0, x1, [sp, #-16]!
	svc 0x2E
	ldp x6, x7, [sp], #16
	stp x1, x2, [x6]
	stp x3, x4, [x6, #16]
	str x5, [x7]
	ret
END_DEF_SVC

DEF_SVC svcGetLastThreadInfo
	stp x1, x2, [sp, #-16]!
	str x0, [sp, #-16]!
	svc 0x2F
	ldr x7, [sp], #16
	stp x1, x2, [x7]
	stp x3, x4, [x7, #16]
	ldp x1, x2, [sp], #16
	str x5, [x1]
	str w6, [x2]
	ret
END_DEF_SVC

DEF_SVC svcGetResourceLimitLimitValue
	str x0, [sp, #-16]!
	svc 0x30
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcGetResourceLimitCurrentValue
	str x0, [sp, #-16]!
	svc 0x31
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSetThreadActivity
	svc 0x32
	ret
END_DEF_SVC

DEF_SVC svcGetThreadContext3
	svc 0x33
	ret
END_DEF_SVC

DEF_SVC svcWaitForAddress
	svc 0x34
	ret
END_DEF_SVC

DEF_SVC svcSignalToAddress
	svc 0x35
	ret
END_DEF_SVC

DEF_SVC svcSynchronizePreemptionState
	svc 0x36
	ret
END_DEF_SVC

DEF_SVC svcDumpInfo
	svc 0x3C
	ret
END_DEF_SVC

DEF_SVC svcKernelDebug
	svc 0x3C
	ret
END_DEF_SVC

DEF_SVC svcChangeKernelTraceState
	svc 0x3D
	ret
END_DEF_SVC

DEF_SVC svcCreateSession
	stp x0, x1, [sp, #-16]!
	svc 0x40
	ldp x3, x4, [sp], #16
	str w1, [x3]
	str w2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcAcceptSession
	str x0, [sp, #-16]!
	svc 0x41
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcReplyAndReceiveLight
	svc 0x42
	ret
END_DEF_SVC

DEF_SVC svcReplyAndReceive
	str x0, [sp, #-16]!
	svc 0x43
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcReplyAndReceiveWithUserBuffer
	str x0, [sp, #-16]!
	svc 0x44
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcCreateEvent
	stp x0, x1, [sp, #-16]!
	svc 0x45
	ldp x3, x4, [sp], #16
	str w1, [x3]
	str w2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcMapPhysicalMemoryUnsafe
	svc 0x48
	ret
END_DEF_SVC

DEF_SVC svcUnmapPhysicalMemoryUnsafe
	svc 0x49
	ret
END_DEF_SVC

DEF_SVC svcSetUnsafeLimit
	svc 0x4A
	ret
END_DEF_SVC

DEF_SVC svcCreateCodeMemory
	str x0, [sp, #-16]!
	svc 0x4B
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcControlCodeMemory
	svc 0x4C
	ret
END_DEF_SVC

DEF_SVC svcSleepSystem
	svc 0x4D
	ret
END_DEF_SVC

DEF_SVC svcReadWriteRegister
	str x0, [sp, #-16]!
	svc 0x4E
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSetProcessActivity
	svc 0x4F
	ret
END_DEF_SVC

DEF_SVC svcCreateSharedMemory
	str x0, [sp, #-16]!
	svc 0x50
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcMapTransferMemory
	svc 0x51
	ret
END_DEF_SVC

DEF_SVC svcUnmapTransferMemory
	svc 0x52
	ret
END_DEF_SVC

DEF_SVC svcCreateInterruptEvent
	str x0, [sp, #-16]!
	svc 0x53
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcQueryPhysicalAddress
	str x0, [sp, #-16]!
	svc 0x54
	ldr x4, [sp], #16
	stp x1, x2, [x4]
	str x3, [x4, #16]
	ret
END_DEF_SVC

DEF_SVC svcQueryIoMapping
	stp x0, x1, [sp, #-16]!
	svc 0x55
	ldp x3, x4, [sp], #16
	str x1, [x3]
	str x2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcLegacyQueryIoMapping
	str x0, [sp, #-16]!
	svc 0x55
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcCreateDeviceAddressSpace
	str x0, [sp, #-16]!
	svc 0x56
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcAttachDeviceAddressSpace
	svc 0x57
	ret
END_DEF_SVC

DEF_SVC svcDetachDeviceAddressSpace
	svc 0x58
	ret
END_DEF_SVC

DEF_SVC svcMapDeviceAddressSpaceByForce
	svc 0x59
	ret
END_DEF_SVC

DEF_SVC svcMapDeviceAddressSpaceAligned
	svc 0x5A
	ret
END_DEF_SVC

DEF_SVC svcMapDeviceAddressSpace
	str x0, [sp, #-16]!
	svc 0x5B
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcUnmapDeviceAddressSpace
	svc 0x5C
	ret
END_DEF_SVC

DEF_SVC svcInvalidateProcessDataCache
	svc 0x5D
	ret
END_DEF_SVC

DEF_SVC svcStoreProcessDataCache
	svc 0x5E
	ret
END_DEF_SVC

DEF_SVC svcFlushProcessDataCache
	svc 0x5F
	ret
END_DEF_SVC

DEF_SVC svcDebugActiveProcess
	str x0, [sp, #-16]!
	svc 0x60
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcBreakDebugProcess
	svc 0x61
	ret
END_DEF_SVC

DEF_SVC svcTerminateDebugProcess
	svc 0x62
	ret
END_DEF_SVC

DEF_SVC svcGetDebugEvent
	svc 0x63
	ret
END_DEF_SVC

DEF_SVC svcLegacyContinueDebugEvent
	svc 0x64
	ret
END_DEF_SVC

DEF_SVC svcContinueDebugEvent
	svc 0x64
	ret
END_DEF_SVC

DEF_SVC svcGetProcessList
	str x0, [sp, #-16]!
	svc 0x65
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcGetThreadList
	str x0, [sp, #-16]!
	svc 0x66
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcGetDebugThreadContext
	svc 0x67
	ret
END_DEF_SVC

DEF_SVC svcSetDebugThreadContext
	svc 0x68
	ret
END_DEF_SVC

DEF_SVC svcQueryDebugProcessMemory
	str x1, [sp, #-16]!
	svc 0x69
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcReadDebugProcessMemory
	svc 0x6A
	ret
END_DEF_SVC

DEF_SVC svcWriteDebugProcessMemory
	svc 0x6B
	ret
END_DEF_SVC

DEF_SVC svcSetHardwareBreakPoint
	svc 0x6C
	ret
END_DEF_SVC

DEF_SVC svcGetDebugThreadParam
	stp x0, x1, [sp, #-16]!
	svc 0x6D
	ldp x3, x4, [sp], #16
	str x1, [x3]
	str w2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcGetSystemInfo
	str x0, [sp, #-16]!
	svc 0x6F
	ldr x2, [sp], #16
	str x1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcCreatePort
	stp x0, x1, [sp, #-16]!
	svc 0x70
	ldp x3, x4, [sp], #16
	str w1, [x3]
	str w2, [x4]
	ret
END_DEF_SVC

DEF_SVC svcManageNamedPort
	str x0, [sp, #-16]!
	svc 0x71
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcConnectToPort
	str x0, [sp, #-16]!
	svc 0x72
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSetProcessMemoryPermission
	svc 0x73
	ret
END_DEF_SVC

DEF_SVC svcMapProcessMemory
	svc 0x74
	ret
END_DEF_SVC

DEF_SVC svcUnmapProcessMemory
	svc 0x75
	ret
END_DEF_SVC

DEF_SVC svcQueryProcessMemory
	str x1, [sp, #-16]!
	svc 0x76
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcMapProcessCodeMemory
	svc 0x77
	ret
END_DEF_SVC

DEF_SVC svcUnmapProcessCodeMemory
	svc 0x78
	ret
END_DEF_SVC

DEF_SVC svcCreateProcess
	str x0, [sp, #-16]!
	svc 0x79
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcStartProcess
	svc 0x7A
	ret
END_DEF_SVC

DEF_SVC svcTerminateProcess
	svc 0x7B
	ret
END_DEF_SVC

DEF_SVC svcGetProcessInfo
	str x0, [sp, #-16]!
	svc 0x7C
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcCreateResourceLimit
	str x0, [sp, #-16]!
	svc 0x7D
	ldr x2, [sp], #16
	str w1, [x2]
	ret
END_DEF_SVC

DEF_SVC svcSetResourceLimitLimitValue
	svc 0x7E
	ret
END_DEF_SVC

DEF_SVC svcCallSecureMonitor
	str x0, [sp, #-16]!
	mov x8, x0
	ldp x0, x1, [x8]
	ldp x2, x3, [x8, #0x10]
	ldp x4, x5, [x8, #0x20]
	ldp x6, x7, [x8, #0x30]
	svc 0x7F
	ldr x8, [sp], #16
	stp x0, x1, [x8]
	stp x2, x3, [x8, #0x10]
	stp x4, x5, [x8, #0x20]
	stp x6, x7, [x8, #0x30]
	ret
END_DEF_SVC