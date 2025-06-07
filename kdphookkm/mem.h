#ifndef MEM_H
#define MEM_H

#include "utils.h"

#define WINDOWS_1803 17134
#define WINDOWS_1809 17763
#define WINDOWS_1903 18362
#define WINDOWS_1909 18363
#define WINDOWS_2004 19041
#define WINDOWS_20H2 19569
#define WINDOWS_21H1 20180

PVOID mem_get_process_base_address(UINT64 pid);

DWORD mem_get_user_directory_table_base_offset(void);

ULONG_PTR mem_get_process_cr3(PEPROCESS process);

ULONG_PTR mem_get_kernel_dir_base(void);

NTSTATUS mem_read_physical_address(
    PVOID target_address,
    PVOID buffer,
    SIZE_T size,
    SIZE_T* bytes_read
);

NTSTATUS mem_write_physical_address(
    PVOID target_address,
    PVOID buffer,
    SIZE_T size,
    SIZE_T* bytes_written
);

uintptr_t mem_translate_linear_address(
    uintptr_t directory_table_base,
    uintptr_t virtual_address
);

NTSTATUS mem_read_virtual(
    uintptr_t dirbase,
    uintptr_t address,
    void* buffer,
    SIZE_T size,
    SIZE_T* bytes_read
);

NTSTATUS mem_write_virtual(
    uintptr_t dirbase,
    uintptr_t address,
    void* buffer,
    SIZE_T size,
    SIZE_T* bytes_written
);

NTSTATUS mem_read_process_memory(
    UINT64 pid,
    PVOID address,
    PVOID buffer,
    SIZE_T size,
    SIZE_T* bytes_read
);

NTSTATUS mem_write_process_memory(
    UINT64 pid,
    PVOID address,
    PVOID buffer,
    SIZE_T size,
    SIZE_T* bytes_written
);

#endif
