#include "ipc.h"

#define printf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__);



NTSTATUS NTAPI DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    
    NTSTATUS status = start_ipc(450009);
    if (status != STATUS_SUCCESS) {
        printf("ipc failed");
        return STATUS_FAIL_CHECK;
    }

    printf("ipc started");

    return status;
}
