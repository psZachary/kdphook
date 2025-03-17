#include <Windows.h>
#include <iostream>

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

    std::cout << "procAddress: " << std::hex << procAddress << std::endl;

    // https://ntdoc.m417z.com/ntusercalculatepopupwindowposition
    using NtUserCalculatePopupWindowPosition_t = BOOL(NTAPI*)(
        _In_ const POINT* anchorPoint,
        _In_ const SIZE* windowSize,
        _In_ ULONG flags,
        _Inout_ RECT* excludeRect,
        _Inout_ RECT* popupWindowPosition
    );
    POINT p{ 10, 10 };
    SIZE f{ 10, 10 };

    RECT z;
    GetWindowRect(GetActiveWindow(), &z);
    RECT x;
    __int64 result = reinterpret_cast<NtUserCalculatePopupWindowPosition_t>(procAddress)(&p, &f, 6969, &z, &x);
    std::cout << "Function returned: " << std::hex << result << std::endl;

    FreeLibrary(moduleHandle);

    return 0;
}

