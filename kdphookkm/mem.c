#include "mem.h"

#define WINDOWS_1803 17134
#define WINDOWS_1809 17763
#define WINDOWS_1903 18362
#define WINDOWS_1909 18363
#define WINDOWS_2004 19041
#define WINDOWS_20H2 19569
#define WINDOWS_21H1 20180

NTKERNELAPI
PVOID
PsGetProcessSectionBaseAddress(
    __in PEPROCESS Process
);

PVOID mem_get_process_base_address(UINT64 pid)
{
    PEPROCESS process = NULL;
    if (pid == 0)
        return 0;

    NTSTATUS nt_status = utils_find_process_pid(pid, &process);
    if (nt_status != STATUS_SUCCESS)
        return 0;

    PVOID base = PsGetProcessSectionBaseAddress(process);
    ObDereferenceObject(process);
    return base;
}

DWORD mem_get_user_directory_table_base_offset()
{
    RTL_OSVERSIONINFOW version = { 0 };
    RtlGetVersion(&version);

    switch (version.dwBuildNumber)
    {
    case WINDOWS_1803:
    case WINDOWS_1809:
        return 0x0278;
    case WINDOWS_1903:
    case WINDOWS_1909:
        return 0x0280;
    case WINDOWS_2004:
    case WINDOWS_20H2:
    case WINDOWS_21H1:
    default:
        return 0x0388;
    }
}

ULONG_PTR mem_get_process_cr3(PEPROCESS process)
{
    PUCHAR process_base = (PUCHAR)process;
    ULONG_PTR dirbase = *(PULONG_PTR)(process_base + 0x28); // x64 dirbase

    if (dirbase == 0)
    {
        DWORD user_dir_offset = mem_get_user_directory_table_base_offset();
        ULONG_PTR user_dirbase = *(PULONG_PTR)(process_base + user_dir_offset);
        return user_dirbase;
    }

    return dirbase;
}

ULONG_PTR mem_get_kernel_dir_base()
{
    PUCHAR current_process = (PUCHAR)PsGetCurrentProcess();
    ULONG_PTR cr3 = *(PULONG_PTR)(current_process + 0x28); // x64 dirbase
    return cr3;
}

NTSTATUS mem_read_physical_address(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read)
{
    MM_COPY_ADDRESS addr_to_read = { 0 };
    addr_to_read.PhysicalAddress.QuadPart = (LONGLONG)target_address;
    return MmCopyMemory(buffer, addr_to_read, size, MM_COPY_MEMORY_PHYSICAL, bytes_read);
}

NTSTATUS mem_write_physical_address(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_written)
{
    if (!target_address)
        return STATUS_UNSUCCESSFUL;

    PHYSICAL_ADDRESS addr_to_write = { 0 };
    addr_to_write.QuadPart = (LONGLONG)target_address;

    PVOID mapped_mem = MmMapIoSpaceEx(addr_to_write, size, PAGE_READWRITE);
    if (!mapped_mem)
        return STATUS_UNSUCCESSFUL;

    memcpy(mapped_mem, buffer, size);

    *bytes_written = size;
    MmUnmapIoSpace(mapped_mem, size);
    return STATUS_SUCCESS;
}

#define PAGE_OFFSET_SIZE 12
static const uintptr_t PAGE_MASK = (~0xfull << 8) & 0xfffffffffull;

uintptr_t mem_translate_linear_address(uintptr_t dirbase, uintptr_t virtual_address)
{
    dirbase &= ~0xf;

    uintptr_t page_offset = virtual_address & ~(~0ul << PAGE_OFFSET_SIZE);
    uintptr_t pte = (virtual_address >> 12) & 0x1ff;
    uintptr_t pt = (virtual_address >> 21) & 0x1ff;
    uintptr_t pd = (virtual_address >> 30) & 0x1ff;
    uintptr_t pdp = (virtual_address >> 39) & 0x1ff;

    SIZE_T read_size = 0;
    uintptr_t pdpe = 0;
    mem_read_physical_address((PVOID)(dirbase + 8 * pdp), &pdpe, sizeof(pdpe), &read_size);
    if (~pdpe & 1)
        return 0;

    uintptr_t pde = 0;
    mem_read_physical_address((PVOID)((pdpe & PAGE_MASK) + 8 * pd), &pde, sizeof(pde), &read_size);
    if (~pde & 1)
        return 0;

    if (pde & 0x80) // 1GB large page
        return (pde & (~0ull << 42 >> 12)) + (virtual_address & ~(~0ull << 30));

    uintptr_t pte_addr = 0;
    mem_read_physical_address((PVOID)((pde & PAGE_MASK) + 8 * pt), &pte_addr, sizeof(pte_addr), &read_size);
    if (~pte_addr & 1)
        return 0;

    if (pte_addr & 0x80) // 2MB large page
        return (pte_addr & PAGE_MASK) + (virtual_address & ~(~0ull << 21));

    uintptr_t final_addr = 0;
    mem_read_physical_address((PVOID)((pte_addr & PAGE_MASK) + 8 * pte), &final_addr, sizeof(final_addr), &read_size);
    final_addr &= PAGE_MASK;

    if (!final_addr)
        return 0;

    return final_addr + page_offset;
}

NTSTATUS mem_read_virtual(uintptr_t dirbase, uintptr_t address, void* buffer, SIZE_T size, SIZE_T* bytes_read)
{
    uintptr_t phys_address = mem_translate_linear_address(dirbase, address);
    return mem_read_physical_address((PVOID)phys_address, buffer, size, bytes_read);
}

NTSTATUS mem_write_virtual(uintptr_t dirbase, uintptr_t address, void* buffer, SIZE_T size, SIZE_T* bytes_written)
{
    uintptr_t phys_address = mem_translate_linear_address(dirbase, address);
    return mem_write_physical_address((PVOID)phys_address, buffer, size, bytes_written);
}

NTSTATUS mem_read_process_memory(UINT64 pid, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read)
{
    PEPROCESS process = NULL;
    if (pid == 0)
        return STATUS_UNSUCCESSFUL;

    NTSTATUS nt_status = PsLookupProcessByProcessId((HANDLE)pid, &process);
    if (nt_status != STATUS_SUCCESS)
        return nt_status;

    ULONG_PTR dirbase = mem_get_process_cr3(process);
    ObDereferenceObject(process);

    SIZE_T cur_offset = 0;
    SIZE_T remaining = size;

    while (remaining > 0)
    {
        uintptr_t phys_address = mem_translate_linear_address(dirbase, (ULONG64)address + cur_offset);
        if (!phys_address)
            return STATUS_UNSUCCESSFUL;

        ULONG64 chunk_size = min(PAGE_SIZE - (phys_address & 0xFFF), remaining);
        SIZE_T chunk_read = 0;

        nt_status = mem_read_physical_address((PVOID)phys_address, (PVOID)((ULONG64)buffer + cur_offset), chunk_size, &chunk_read);
        if (nt_status != STATUS_SUCCESS || chunk_read == 0)
            break;

        remaining -= chunk_read;
        cur_offset += chunk_read;
    }

    if (bytes_read != 0x0)
        *bytes_read = cur_offset;
    return nt_status;
}

NTSTATUS mem_write_process_memory(UINT64 pid, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* bytes_written)
{
    PEPROCESS process = NULL;
    if (pid == 0)
        return STATUS_UNSUCCESSFUL;

    NTSTATUS nt_status = utils_find_process_pid(pid, &process);
    if (nt_status != STATUS_SUCCESS)
        return nt_status;

    ULONG_PTR dirbase = mem_get_process_cr3(process);
    ObDereferenceObject(process);

    SIZE_T cur_offset = 0;
    SIZE_T remaining = size;

    while (remaining > 0)
    {
        uintptr_t phys_address = mem_translate_linear_address(dirbase, (ULONG64)address + cur_offset);
        if (!phys_address)
            return STATUS_UNSUCCESSFUL;

        ULONG64 chunk_size = min(PAGE_SIZE - (phys_address & 0xFFF), remaining);
        SIZE_T chunk_written = 0;

        nt_status = mem_write_physical_address((PVOID)phys_address, (PVOID)((ULONG64)buffer + cur_offset), chunk_size, &chunk_written);
        if (nt_status != STATUS_SUCCESS || chunk_written == 0)
            break;

        remaining -= chunk_written;
        cur_offset += chunk_written;
    }
    if (bytes_written != 0x0)
        *bytes_written = cur_offset;
    return nt_status;
}
