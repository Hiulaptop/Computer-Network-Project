#pragma once
#include "Screen.hpp"
#include "string"

class StartScreen : public Screen
{
private:
    char m_IP[32] = {};
    float m_timeout = 0.0f;
public:
    StartScreen(Core &core);

    void Render(float DT) override;

    void Init() override;

    void OnExit() override;
};