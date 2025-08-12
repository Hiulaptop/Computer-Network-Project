#pragma once
#include "Screen.hpp"
#include "string"

class StartScreen : public Screen
{
private:
    char m_IP[128];

public:
    explicit StartScreen(Core &core);

    void Render(float DT) override;

    void Init() override;

    void OnExit() override;

};