#pragma once
#include <string>
#include <unordered_map>
#include <winsock2.h>

#include "HandleFeature.hpp"



class SubMenu {
protected:
    std::string m_Name;
public:
    virtual ~SubMenu() = default;

    virtual void RenderLeft(float DT) = 0;
    virtual void RenderRight(float DT) = 0;
    void Render(float DT);
};
class ProcessSubMenu : public SubMenu {
    ProcessFeature m_processFeature;
    float timer = 0;
public:
    ProcessSubMenu(char* IP);

    void RenderLeft(float DT) override;

    void RenderRight(float DT) override;
};
class FileSubMenu : public SubMenu {
    FileFeature m_FileFeature;
public:
    FileSubMenu(char* IP);

    void RenderLeft(float DT) override;

    void RenderRight(float DT) override;
};
class KeylogSubMenu : public SubMenu {
    KeyloggerFeature m_KeyloggerFeature;
public:
    KeylogSubMenu(char* IP);

    void RenderLeft(float DT) override;

    void RenderRight(float DT) override;
};
class WinUtilsSubMenu : public SubMenu {
    WindowFeature m_WindowFeature;
public:
    WinUtilsSubMenu(char* IP);

    void RenderLeft(float DT) override;

    void RenderRight(float DT) override;
};
class WebcamSubMenu : public SubMenu {
    VideoFeature m_VideoFeature;
public:
    WebcamSubMenu(char* IP);

    void RenderLeft(float DT) override;

    void RenderRight(float DT) override;
};