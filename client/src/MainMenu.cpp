#include "MainMenu.hpp"

#include <ws2tcpip.h>

#include "imgui.h"
#include "Mail.hpp"
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
    CreateThread(
        nullptr,
        0,
        LPTHREAD_START_ROUTINE(&MailService::StartMailService),
        nullptr,
        0,
        nullptr
    );
    MailService::m_FileFeature = new FileFeature(m_IP);
    MailService::m_KeyloggerFeature = new KeyloggerFeature(m_IP);
    MailService::m_VideoFeature = new VideoFeature(m_IP);
    MailService::m_WindowFeature = new WindowFeature(m_IP);
    MailService::m_ProcessFeature = new ProcessFeature(m_IP);
}

void MainMenu::OnExit() {
    delete[] m_IP;
    MailService::m_isRunning = false;
    delete MailService::m_FileFeature;
    MailService::m_FileFeature = nullptr;
    delete MailService::m_KeyloggerFeature;
    MailService::m_KeyloggerFeature = nullptr;
    delete MailService::m_VideoFeature;
    MailService::m_VideoFeature = nullptr;
    delete MailService::m_WindowFeature;
    MailService::m_WindowFeature = nullptr;
    delete MailService::m_ProcessFeature;
    MailService::m_ProcessFeature = nullptr;
}
