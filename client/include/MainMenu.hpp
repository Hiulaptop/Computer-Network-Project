#pragma once
#include "Screen.hpp"
#include "SubMenu.hpp"
#include <winsock2.h>

class MainMenu : public Screen {
private:
    SOCKET m_Socket;
    char* m_IP = nullptr;
    ProcessSubMenu processSubMenu;
    FileSubMenu fileSubMenu;
    KeylogSubMenu keylogSubMenu;
    WinUtilsSubMenu winUtilsSubMenu;
    WebcamSubMenu webcamSubMenu;
public:
    MainMenu(Core& core, SOCKET socket);

    MainMenu(Core &core, const char *ip);

    void Render(float DT) override;

    void Init() override;

    void OnExit() override;
};
