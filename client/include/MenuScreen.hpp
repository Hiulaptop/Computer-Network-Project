#pragma once
#include "Screen.hpp"
#include "string"

enum RequestTypes {
    KeyLog,
    Record,
    ViewProcess,
    StartProcess,
    StopProcess,
    GetFile,
    IncreaseVolume,
    DecreaseVolume,
    TurnOffDevice,
    RestartDevice,
    Exit,
    None
};

class MenuScreen : public Screen
{
private:
    RequestTypes m_RequestType;
public:
    explicit MenuScreen(Core &core): Screen(core) {
        m_RequestType = RequestTypes::None;
    };

    void Render(float DT) override;

    void Init() override;

    void OnExit() override;
};