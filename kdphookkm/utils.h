#ifndef UTILS_H
#define UTILS_H

#include <ntifs.h>
#include <ntstrsafe.h>

#define printf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__);
#define isbadkptr(ptr) (!ptr || (ULONGLONG)ptr < 0xFFFF000000000000)


static PVOID utils_find_pattern(PVOID base, SIZE_T srch_len, const UCHAR* pattr, SIZE_T pattr_size) {
    UCHAR* s = (UCHAR*)base;
    UCHAR* e = s + srch_len - pattr_size;

    for (UCHAR* addr = s; addr <= e; addr++) {
        SIZE_T i;
        for (i = 0; i < pattr_size; i++) {
            if (pattr[i] != 0x00 && addr[i] != pattr[i]) {
                break;
            }
        }
        if (i == pattr_size) {
            return addr;
        }
    }

    return NULL;
}

typedef struct _SYSTEM_MODULE {
    PVOID Reserved[2];
    PVOID Base;
    ULONG Size;
    ULONG Flags;
    USHORT Index;
    USHORT Unknown;
    USHORT LoadCount;
    USHORT ModuleNameOffset;
    CHAR ImageName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION {
    ULONG ModuleCount;
    SYSTEM_MODULE Modules[256];  // Fixed-size buffer
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemModuleInformation = 11
} SYSTEM_INFORMATION_CLASS;

typedef unsigned __int64 QWORD;
typedef unsigned char BYTE;

extern NTSTATUS ZwQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

static PVOID utils_get_module_base(PUCHAR module, OUT SIZE_T* psize) {
    ULONG module_info_size = 0;
    ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &module_info_size);

    PSYSTEM_MODULE_INFORMATION pmodule_info = (PSYSTEM_MODULE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, module_info_size, 'modB');
    if (!pmodule_info) return NULL;

    if (NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, pmodule_info, module_info_size, &module_info_size))) {
        for (ULONG i = 0; i < pmodule_info->ModuleCount; i++) {

            if (strcmp((const char*)module, (const char*)(pmodule_info->Modules[i].ImageName + pmodule_info->Modules[i].ModuleNameOffset)) == 0) {
                PVOID base = pmodule_info->Modules[i].Base;
                if (psize)
                    *psize = pmodule_info->Modules[i].Size;
                ExFreePoolWithTag(pmodule_info, 'modb');
                return base;
            }
        }
    }

    ExFreePoolWithTag(pmodule_info, 'modb');
    return NULL;
}


static NTSTATUS utils_find_process(PUCHAR process_name, PEPROCESS* process)
{
    PEPROCESS sys_process = PsInitialSystemProcess;
    PEPROCESS curr_entry = sys_process;

    char image_name[15];

    do {
        RtlCopyMemory((PVOID)(&image_name), (PVOID)((uintptr_t)curr_entry + 0x5a8), sizeof(image_name));

        if (strstr(image_name, (const char*)process_name)) {
            DWORD active_threads;
            RtlCopyMemory((PVOID)&active_threads, (PVOID)((uintptr_t)curr_entry + 0x5f0), sizeof(active_threads));
            if (active_threads) {
                *process = curr_entry;
                return STATUS_SUCCESS;
            }
        }

        PLIST_ENTRY list = (PLIST_ENTRY)((uintptr_t)(curr_entry)+0x448);
        curr_entry = (PEPROCESS)((uintptr_t)list->Flink - 0x448);

    } while (curr_entry != sys_process);

    return STATUS_NOT_FOUND;
}

#endif