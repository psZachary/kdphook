#include "hook.h"

NTSTATUS hook_via_data_ptr(
    PUCHAR export_module, 
    PUCHAR pattern, 
    UINT32 pattern_size,
    UINT32 pattern_offset,
    PUCHAR target_process_name, 
    PVOID hook_function, 
    PVOID original
)
{
    PEPROCESS target_process        = 0x0;
    ULONGLONG target_process_id     = 0x0;
    SIZE_T module_size              = 0x0;
    PVOID module_base               = 0x0;
    UINT64 data_ptr_call            = 0x0;
    UINT64 data_ptr                 = 0x0;


    module_base = utils_get_module_base(export_module, &module_size);
    if (isbadkptr(module_base)) {
        printf("%s base is invalid: %p", export_module, module_base);
        return STATUS_DATA_ERROR;
    }
    printf("%s base is valid: %p", export_module, module_base);


    data_ptr_call = (UINT64)utils_find_pattern(module_base, module_size, pattern, pattern_size) + pattern_offset;
    if (isbadkptr(data_ptr_call)) {
        printf("data_ptr_call invalid: %llx", data_ptr_call);
        return STATUS_DATA_ERROR;
    }
    printf("data_ptr_call valid: %llx", data_ptr_call);


    data_ptr = (UINT64)data_ptr_call + *(int*)((UINT8*)data_ptr_call + 3) + 7;
    if (isbadkptr(data_ptr)) {
        printf("nt_data_ptr invalid: %llx", data_ptr);
        return STATUS_DATA_ERROR;
    }
    printf("nt_data_ptr valid: %llx", data_ptr);


    utils_find_process(target_process_name, &target_process);
    if (!target_process) {
        printf("invalid PEPROESS: %p", target_process);
        return STATUS_DATA_ERROR;
    }
    printf("valid PEPROESS: %p", target_process);

    target_process_id = (ULONGLONG)PsGetProcessId(target_process);
    if (!target_process_id) {
        printf("target_process_id null: %llu", target_process_id);
        return STATUS_DATA_ERROR;
    }
    printf("target_process_id valid: %llu", target_process_id);

    KeAttachProcess(target_process);
    *(void**)&original = _InterlockedExchangePointer((PVOID*)data_ptr, (PVOID)hook_function);
    KeDetachProcess();
    printf("finished hooking, returning");

	return STATUS_SUCCESS;
}
