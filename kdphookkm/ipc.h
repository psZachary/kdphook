#ifndef IPC_H
#define IPC_H

#include "hook.h"
#include <windef.h>

typedef unsigned int IPC_COMM_SECRET, PID;

typedef enum _IPC_COMM_TYPE {
    SETUP,
    BASIC_REPEAT_SECRET,
    READ_PROCESS,
    WRITE_PROCESS
} IPC_COMM_TYPE;

typedef struct _IPC_COMM {
    IPC_COMM_TYPE type;
    PID pid;
    UINT64 addr_from;
    UINT64 addr_to;
    SIZE_T size;

} IPC_COMM, *PIPC_COMM;

#define IPC_COMM_SUCCESS 1;
#define IPC_COMM_FAILURE 0;

NTSTATUS start_ipc(IPC_COMM_SECRET ipc_comm_secret);

#endif
