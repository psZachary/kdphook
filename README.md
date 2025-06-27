# kdphook
### Warning
Any anti-cheat or anti-malware program can detect this driver running in unsigned memory through NMI callbacks / stack walks (if mapped to unsigned memory), this can be circumvented by signing the driver with a cert or fancy method to make the stack look legit. 
### Usage Example
```c++
#include <Windows.h>
#include <iostream>
#include <thread>
#include "driver.hpp" 

int main() {
    driver* drv = new driver();

    // Allocate new driver object on the heap
    if (!drv) {
        printf("failed to create driver object");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 1;
    }

    // Initialize the driver by Loading win32 library and getting proc address of IPC routine
    if (!drv->initialize(450009)) {
        printf("failed to create driver object");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 1;
    }

    // Simple check if IPC comm is established
    if (!drv->is_ready()) {
        printf("failed to check if ready");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ereturn 1;
    }

    // Attach to target process with process ID
    drv->attach(9228);

    char buffer[64]{};
    // Read virtual memory of process (9228) and store it into the buffer
    drv->read_process_memory(0x7ff62e090000, &buffer[0], sizeof(buffer));

    printf("buffer: %s", buffer);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
```
### Credits
[mem.c](https://www.unknowncheats.me/forum/anti-cheat-bypass/444289-read-process-physical-memory-attach.html)
