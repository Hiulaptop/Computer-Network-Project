#include "SubMenu.hpp"

#include "imgui.h"
#include "imgui_internal.h"

void SubMenu::Render() {
    if (ImGui::BeginTabItem(m_Name.c_str(), nullptr, ImGuiTabItemFlags_None)) {
        RenderLeft();
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        RenderRight();
        ImGui::EndTabItem();
    }
}

ProcessSubMenu::ProcessSubMenu() {
    m_Name = "Process Management";
}

void ProcessSubMenu::RenderLeft() {
    ImGui::BeginChild("ListFeatures", ImVec2(200, -1), true);
    ImGui::Button("Reload Process List", ImVec2(-1, 44));
    ImGui::BeginDisabled();
    ImGui::Button("Stop Process", ImVec2(-1, 44));
    ImGui::Button("Start Process", ImVec2(-1, 44));
    ImGui::EndDisabled();
    ImGui::EndChild();
}

void ProcessSubMenu::RenderRight() {
    if (ImGui::BeginTable("ProcessTable1", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        // ImGui::BeginChild("ProcessTableChild", ImVec2(-1, -1), true);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Parent PID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Memory Usage", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("CPU Time", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        // ImGui::EndChild();
        ImGui::EndTable();
    }
}

FileSubMenu::FileSubMenu() {
    m_Name = "File Management";
}

void FileSubMenu::RenderLeft() {
    ImGui::BeginChild("ListFeatures", ImVec2(200, -1), true);
    ImGui::BeginDisabled();
    ImGui::Button("Open/Download", ImVec2(-1, 44));
    ImGui::EndDisabled();
    ImGui::EndChild();
}

void FileSubMenu::RenderRight() {
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

KeylogSubMenu::KeylogSubMenu() {
    m_Name = "Keylogger";
}

void KeylogSubMenu::RenderLeft() {
    ImGui::BeginChild("KeylogFeatures", ImVec2(200, -1), true);
    ImGui::Button("Start Keylogger", ImVec2(-1, 44));
    ImGui::EndChild();
}

void KeylogSubMenu::RenderRight() {
    ImGui::BeginChild("KeylogOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped("Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.Keylogger is not implemented yet.");
    ImGui::EndChild();
}
WinUtilsSubMenu::WinUtilsSubMenu() {
    m_Name = "Windows Utilities";
}

void WinUtilsSubMenu::RenderLeft() {
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

void WinUtilsSubMenu::RenderRight() {
    ImGui::BeginChild("WinUtilsOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped("Windows Utilities are not implemented yet. Windows Utilities are not implemented yet. Windows Utilities are not implemented yet.");
    ImGui::EndChild();
}

WebcamSubMenu::WebcamSubMenu() {
    m_Name = "Webcam";
}

void WebcamSubMenu::RenderLeft() {
    ImGui::BeginChild("WebcamFeatures", ImVec2(200, -1), true);
    ImGui::Button("Start Webcam", ImVec2(-1, 44));
    ImGui::Button("Download Webcam", ImVec2(-1, 44));
    ImGui::EndChild();
}

void WebcamSubMenu::RenderRight() {
    ImGui::BeginChild("WebcamOutput", ImVec2(-1, -1), true);
    ImGui::TextWrapped("Webcam is not implemented yet. Webcam is not implemented yet");
    ImGui::EndChild();
}
