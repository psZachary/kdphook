#include <Windows.h>
#include <iostream>
#include <thread>
#include "driver.hpp" 

int main() {
    driver* drv = new driver();
    if (!drv) {
        printf("failed to create driver object");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        exit(1);
    }

    if (!drv->initialize(450009)) {
        printf("failed to create driver object");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        exit(1);
    }

    if (!drv->is_ready()) {
        printf("failed to check if ready");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        exit(1);
    }

    char buffer[64]{};
    drv->attach(9228);
    drv->read_process_memory(0x7ff62e090000, &buffer[0], sizeof(buffer));

    printf("buffer: %s", buffer);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
