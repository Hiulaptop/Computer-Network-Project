#include <iostream>
// #include <winsock2.h>
// #include <ws2tcpip.h>

#include "UICore.hpp"

int main() {
    Core core;
    core.Init();
    core.Start();
    core.Shutdown();

    return 0;
}
