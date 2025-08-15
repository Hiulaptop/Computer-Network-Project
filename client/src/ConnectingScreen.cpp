#include "ConnectingScreen.hpp"

#include "UICore.hpp"

void ConnectingScreen::Render(float DT) {
    m_totalTime += DT;
    auto *drawList = ImGui::GetWindowDrawList();
    drawList->PathClear();
    ImGui::SetCursorPos(m_CirclePos - ImVec2(ImGui::CalcTextSize("Disconnect").x/2, CIRCLE_RADIUS + 5)* 2.5f);
    ImGui::Text("Connecting to %s...", m_IP);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        const float progress = 1.0f - (float)i / (SAMPLE_COUNT - 1);
        const float head_angle = m_totalTime * SPINNER_SPEED;
        const float tail_offset_angle = ((float)i / (SAMPLE_COUNT - 1)) * ARC_LENGTH;
        const float final_angle = head_angle - tail_offset_angle;
        const float dot_radius = DOT_RADIUS_MIN + (DOT_RADIUS_MAX - DOT_RADIUS_MIN) * progress;
        const unsigned char alpha = (unsigned char)(50 + 205 * progress);
        ImVec2 circlePos = ImVec2(m_CirclePos.x + CIRCLE_RADIUS * cosf(final_angle),
                                  m_CirclePos.y + CIRCLE_RADIUS * sinf(final_angle));

        drawList->AddCircleFilled(circlePos, dot_radius, IM_COL32(255, 255, 255, alpha));
    }
    ImGui::SetCursorPos(m_CirclePos + ImVec2(-ImGui::CalcTextSize("Cancel").x / 2 - 4, CIRCLE_RADIUS * 2.5f));
    if (ImGui::Button("Cancel")) {
        m_Core.PopScreen();
    }
}

void ConnectingScreen::Init() {
}

void ConnectingScreen::OnExit() {
    delete[] m_IP;
    m_IP = nullptr;
}