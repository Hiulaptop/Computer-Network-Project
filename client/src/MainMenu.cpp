#include "MainMenu.hpp"

#include <ws2tcpip.h>

#include "imgui.h"
#include "StartScreen.hpp"
#include "UICore.hpp"

MainMenu::MainMenu(Core &core, char *IP): Screen(core), processSubMenu(IP), fileSubMenu(IP),
                                               keylogSubMenu(IP),
                                               winUtilsSubMenu(IP), webcamSubMenu(IP) {
    m_IP = strdup(IP);
}

void MainMenu::Render(float DT) {
    ImGui::Text("Welcome to the Main Menu!");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize("Disconnect").x + m_Core.GetStyle().FramePadding.x));
    if (ImGui::Button("Disconnect")) {
        m_Core.PopScreen();
    }
    ImGui::Text("Server IP: %s", m_IP);
    ImGui::SeparatorText("Options");
    ImGui::BeginTabBar("Control", ImGuiTabBarFlags_None);
    processSubMenu.Render(DT);
    fileSubMenu.Render(DT);
    keylogSubMenu.Render(DT);
    winUtilsSubMenu.Render(DT);
    webcamSubMenu.Render(DT);
    ImGui::EndTabBar();
}

void MainMenu::Init() {
    m_Core.GetStyle().FontScaleDpi = 1;
}

void MainMenu::OnExit() {
    delete[] m_IP;
}
