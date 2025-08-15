#pragma once
#include <cstring>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <math.h>

#include "imgui.h"
#include "Screen.hpp"
#include "UICore.hpp"
class ConnectingScreen : public Screen {
    char *m_IP = nullptr;
    const ImVec2 m_CirclePos = m_Core.GetIO().DisplaySize * 0.5f;
    const int SAMPLE_COUNT = 20;
    float m_totalTime = 0.0f;
    const float CIRCLE_RADIUS = 30.0f;
    const float DOT_RADIUS_MIN  = 2.0f;
    const float DOT_RADIUS_MAX  = 5.0f;
    const float SPINNER_SPEED   = 5.0f;
    const float ARC_LENGTH      = M_PI * 1.5f;
public:
    ConnectingScreen(Core &core, const char *ip): Screen(core) {
        m_IP = strdup(ip);
        m_IP[strlen(m_IP)] = '\0';
    }
    void Render(float DT) override;

    void Init() override;

    void OnExit() override;
};
