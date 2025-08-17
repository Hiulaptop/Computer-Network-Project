#include "MenuScreen.hpp"
#include <iostream>

#include "UICore.hpp"

void MenuScreen::Render(float DT) {
    std::string req_text[] = {
        "KeyLog",
        "Record",
        "Start Process",
        "Stop Process",
        "Get File",
        "Increase Volume",
        "Decrease Volume",
        "Turn Off Device",
        "Restart Device",
        "Exit",
    };
    if (ImGui::Button(req_text[0].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[0] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[1].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[1] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[2].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[2] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[3].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[3] << std::endl;
    }
    if (ImGui::Button(req_text[4].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[4] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[5].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[5] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[6].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[6] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[7].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[7] << std::endl;
    }
    if (ImGui::Button(req_text[8].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[8] << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button(req_text[9].c_str(), ImVec2(300, 230))) {
        std::cout << req_text[9] << std::endl;
    }
}

void MenuScreen::Init() {
}

void MenuScreen::OnExit() {
}
