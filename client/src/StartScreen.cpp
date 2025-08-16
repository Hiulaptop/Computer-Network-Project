#include "ConnectingScreen.hpp"
#include "StartScreen.hpp"
#include <iostream>
#include <ws2tcpip.h>
#include <MainMenu.hpp>

#include "UICore.hpp"

StartScreen::StartScreen(Core &core): Screen(core) {
    memset(m_IP, 0, sizeof(m_IP));
}

void StartScreen::Render(float DT) {
    IN_ADDR addr;
    bool isValid = inet_pton(AF_INET, m_IP, &addr) == 1;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    if (isValid) {
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(52,178,51, 255));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255,0,0,255));
    }
    ImGui::SetCursorPos(m_Core.GetIO().DisplaySize/2.5f - ImVec2(ImGui::CalcTextSize("Please Enter Server IP Here: ").x/2, ImGui::CalcTextSize("Please Enter Server IP Here: ").y ));
    ImGui::LabelText("##ipLabel", "Please Enter Server IP Here:");
    ImGui::SetCursorPos(m_Core.GetIO().DisplaySize/2.5f - ImVec2(ImGui::CalcTextSize("Please Enter Server IP Here: ").x/2, -10 ));
    ImGui::PushItemWidth(500);
    if (m_timeout > 0 || m_IP[0] == '\0') {
        ImGui::SetKeyboardFocusHere(0);
        m_Core.GetStyle().FontScaleDpi = 1.5f;
    }
    bool isEntered = ImGui::InputTextWithHint("##IPaddress", "0.0.0.0", m_IP, IM_ARRAYSIZE(m_IP), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::SetCursorPos(m_Core.GetIO().DisplaySize/2.5f - ImVec2(-250 , - ImGui::GetTextLineHeightWithSpacing() * 2));
    if (ImGui::Button("Connect") || isEntered) {
        if (isValid) {
            m_Core.PushScreen<ConnectingScreen>(m_Core, m_IP);
            memset(m_IP, 0, sizeof(m_IP));
        } else {
            m_timeout = 1.0f;
        }
    }
    if (m_timeout > 0) {
        m_timeout -= DT;
        ImGui::SetCursorPosX(m_Core.GetIO().DisplaySize.x/2.5f - ImGui::CalcTextSize("Please enter a valid IP address.").x / 2.5f - 15);
        ImGui::Text("Please enter a valid IP address.");
    }
    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter() - ImVec2(0,50), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopup("Connection Error")) {
        ImGui::Text("Failed to connect to the server. Please check your IP address.");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void StartScreen::Init() {
    m_Core.GetStyle().FontScaleDpi = 1.5f;
}

void StartScreen::OnExit() {
    std::cerr << "Exiting..." << std::endl;
}
