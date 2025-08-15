#pragma once
#include <string>

class SubMenu {
protected:
    std::string m_Name;
public:
    virtual ~SubMenu() = default;

    virtual void RenderLeft() = 0;
    virtual void RenderRight() = 0;
    void Render();
};
class ProcessSubMenu : public SubMenu {
public:
    ProcessSubMenu();

    void RenderLeft() override;

    void RenderRight() override;
};
class FileSubMenu : public SubMenu {
public:
    FileSubMenu();

    void RenderLeft() override;

    void RenderRight() override;
};
class KeylogSubMenu : public SubMenu {
public:
    KeylogSubMenu();

    void RenderLeft() override;

    void RenderRight() override;
};
class WinUtilsSubMenu : public SubMenu {
public:
    WinUtilsSubMenu();

    void RenderLeft() override;

    void RenderRight() override;
};
class WebcamSubMenu : public SubMenu {
public:
    WebcamSubMenu();

    void RenderLeft() override;

    void RenderRight() override;
};