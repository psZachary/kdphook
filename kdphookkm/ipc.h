#ifndef IPC_H
#define IPC_H

#include "hook.h"
#include <windef.h>

#define printf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__);

typedef unsigned long IPC_COMM_SECRET;
#define IPC_COMM_SUCCESS 1;
#define IPC_COMM_FAILURE 0;

NTSTATUS start_ipc(IPC_COMM_SECRET ipc_comm_secret);

#endif
