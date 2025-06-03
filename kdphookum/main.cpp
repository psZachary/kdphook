#include <Windows.h>
#include <iostream>
#include "ipc.hpp"  // Ensure ipc.hpp is included BEFORE using IPC_COMM_OPERATION

int main() {
    // Load the win32u.dll module
    HMODULE moduleHandle = LoadLibraryA("win32u.dll");
    if (!moduleHandle) {
        std::cerr << "Failed to load win32u.dll!" << std::endl;
        return 1;
    }

    std::cout << "moduleHandle: " << std::hex << moduleHandle << std::endl;

    auto procAddress = GetProcAddress(moduleHandle, "NtUserCalculatePopupWindowPosition");
    if (!procAddress) {
        std::cerr << "Failed to locate function!" << std::endl;
        FreeLibrary(moduleHandle);
        return 1;
    }


    // https://ntdoc.m417z.com/ntusercalculatepopupwindowposition
    using NtUserCalculatePopupWindowPosition_t = BOOL(NTAPI*)(
        _In_ const POINT* anchorPoint,
        _In_ const SIZE* windowSize,
        _In_ ULONG flags,
        _Inout_ RECT* excludeRect,
        _Inout_ RECT* popupWindowPosition
        );

    using NtHookIPC_t = BOOL(NTAPI*)(
        _In_    const IPC_COMM_SECRET secret
    );


   
    __int64 result = reinterpret_cast<NtHookIPC_t>(procAddress)(6969);
    printf("%d", result);
    FreeLibrary(moduleHandle);
    return 0;
}
