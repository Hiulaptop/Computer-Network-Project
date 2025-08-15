#include "MainMenu.hpp"

#include <ws2tcpip.h>

#include "imgui.h"
#include "StartScreen.hpp"
#include "UICore.hpp"

MainMenu::MainMenu(Core &core, SOCKET socket): Screen(core), m_Socket(socket) {
}

MainMenu::MainMenu(Core &core, const char *ip): Screen(core), m_Socket(INVALID_SOCKET) {
    assert(ip != nullptr);
    m_IP = strdup(ip);
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
    processSubMenu.Render();
    fileSubMenu.Render();
    keylogSubMenu.Render();
    winUtilsSubMenu.Render();
    // webcamSubMenu.Render();
    ImGui::EndTabBar();
}

void MainMenu::Init() {
    m_Core.GetStyle().FontScaleDpi = 1;
    // SOCKADDR_IN addr = {0};
    // int addrLen = sizeof(addr);
    // getpeername(m_Socket, reinterpret_cast<SOCKADDR *>(&addr), &addrLen);
    // inet_ntop(AF_INET, &addr.sin_addr, m_IP, sizeof(m_IP));
    // m_IP[sizeof(m_IP) - 1] = '\0';
}

void MainMenu::OnExit() {
    delete[] m_IP;
    closesocket(m_Socket);
}
