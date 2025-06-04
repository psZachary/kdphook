#ifndef IPC_H
#define IPC_H

#include "hook.h"
#include <windef.h>

typedef unsigned int IPC_COMM_SECRET, PID;

typedef enum _IPC_COMM_TYPE {
    SETUP,
    BASIC_REPEAT_SECRET,
    PHYSICAL_WRITE,
    PHYSICAL_READ
} IPC_COMM_TYPE;

typedef struct _IPC_COMM {
    PID pid;
    IPC_COMM_TYPE type;
} IPC_COMM, *PIPC_COMM;

#define IPC_COMM_SUCCESS 1;
#define IPC_COMM_FAILURE 0;

NTSTATUS start_ipc(IPC_COMM_SECRET ipc_comm_secret);

#endif
