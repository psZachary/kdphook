#ifndef HOOK_H
#define HOOK_H

#include "utils.h"

NTSTATUS hook_via_data_ptr(PUCHAR export_module, PUCHAR pattern, UINT32 pattern_size, UINT32 pattern_offset, PUCHAR target_process_name, PVOID hook_function, PVOID original);

#endif