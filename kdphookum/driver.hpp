#pragma once

#include <Windows.h>

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

} IPC_COMM, * PIPC_COMM;


#define IPC_COMM_SUCCESS 1;
#define IPC_COMM_FAILURE 0;

typedef BOOL(NTAPI* p_ipc_routine)(unsigned long long, unsigned long long, unsigned int);

class driver {
private:
    bool initialized;
    IPC_COMM_SECRET _secret;
    HMODULE win32u_handle;
    p_ipc_routine ipc_routine;
    unsigned int process_id;

    template<typename T>
    T send_request(IPC_COMM* ipc_comm) {
        if (!ipc_routine || !initialized) return (T)(0);
        return (T)(ipc_routine((uint64_t)ipc_comm, 1LLU, _secret));
    }
public:
    driver() {
        initialized = false;
        _secret = 0;
        win32u_handle = 0;
        ipc_routine = 0;
        process_id = 0;
    };
    void initialize_gui_subsystem() {
        // initialize gui thread so syscall can safely dispatch
        GetForegroundWindow();
    }
    bool initialize(IPC_COMM_SECRET secret) {
        if (secret <= 0) return false;

        _secret = secret;

        win32u_handle = LoadLibraryA("\x77\x69\x6E\x33\x32\x75\x2E\x64\x6C\x6C\x00");
        if (!win32u_handle) return false;

        ipc_routine = reinterpret_cast<p_ipc_routine>(GetProcAddress(win32u_handle, "\x4E\x74\x55\x73\x65\x72\x43\x61\x6C\x63\x75\x6C\x61\x74\x65\x50\x6F\x70\x75\x70\x57\x69\x6E\x64\x6F\x77\x50\x6F\x73\x69\x74\x69\x6F\x6E\x00"));
        if (!ipc_routine) return false;

        initialize_gui_subsystem();

        initialized = true;

        return (ipc_routine && win32u_handle);
    }
    bool is_ready() {
        IPC_COMM ipc_comm{};
        ipc_comm.pid = 0;
        ipc_comm.type = IPC_COMM_TYPE::BASIC_REPEAT_SECRET;
        return send_request<int>(&ipc_comm) == _secret;
    }
    void attach(unsigned int pid) {
        this->process_id = pid;
    }
    bool read_process_memory(uint64_t address, void* buffer, size_t size) {
        IPC_COMM ipc_comm{};
        ipc_comm.pid = this->process_id;
        ipc_comm.type = IPC_COMM_TYPE::READ_PROCESS;
        ipc_comm.addr_from = address;
        ipc_comm.addr_to = (uint64_t)buffer;
        ipc_comm.size = size;
        return send_request<bool>(&ipc_comm);
    }
    bool write_process_memory(uint64_t address, void* buffer, size_t size) {
        IPC_COMM ipc_comm{};
        ipc_comm.pid = this->process_id;
        ipc_comm.type = IPC_COMM_TYPE::WRITE_PROCESS;
        ipc_comm.addr_from = (uint64_t)buffer;
        ipc_comm.addr_to = address;
        ipc_comm.size = size;
        return send_request<bool>(&ipc_comm);
    }
};