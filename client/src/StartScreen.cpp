#include "StartScreen.hpp"
#include <iostream>
#include "UICore.hpp"



StartScreen::StartScreen(Core &core): Screen(core) {
    memset(m_IP, 0, sizeof(m_IP));
}

void StartScreen::Render(float DT) {
    char buffer[128] = {};
    bool tmp = ImGui::InputTextWithHint("IP address", "Please Enter Server IP Here: ", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
    if (buffer[0]) strcpy(m_IP, buffer);
    ImGui::Text("%s", m_IP);
    if (ImGui::Button("Save")) {
        // Connect here

        // PushScreen here

        // PopScreen here
        m_Core.PopScreen();
    }
}

void StartScreen::Init() {
}

void StartScreen::OnExit() {
    std::cerr << "Exiting..." << std::endl;
}
