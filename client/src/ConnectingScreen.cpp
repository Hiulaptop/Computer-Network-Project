#include "ConnectingScreen.hpp"

#include <complex.h>
#include <iostream>

#include "MainMenu.hpp"
#include "UICore.hpp"

void ConnectingScreen::Render(float DT) {
    if (m_totalTime > 5.f) {
        m_Core.PopScreen();
        ImGui::OpenPopup("Connection Error");
        std::cerr << "Connection timed out after 5 seconds." << std::endl;
        return;
    }
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
    fd_set writeFds, exceptFds;
    FD_ZERO(&writeFds);
    FD_ZERO(&exceptFds);
    FD_SET(m_Socket, &writeFds);
    FD_SET(m_Socket, &exceptFds);

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(0, nullptr, &writeFds, &exceptFds, &timeout);
    if (result < 0) {
        m_Core.PopScreen();
        ImGui::OpenPopup("Connection Error");
        std::cerr << "Select failed: " << WSAGetLastError() << std::endl;
        return;
    }
    if (result > 0) {
        if (FD_ISSET(m_Socket, &exceptFds)) {
            int errorCode = WSAGetLastError();
            m_Core.PopScreen();
            ImGui::OpenPopup("Connection Error");
            std::cerr << "Socket error: " << errorCode << std::endl;
            closesocket(m_Socket);
            return;
        }
        if (FD_ISSET(m_Socket, &writeFds)) {

            m_Core.ChangeScreen<MainMenu>(m_Core, m_Socket);
        }
    }
}

void ConnectingScreen::Init() {
    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET) {
        m_Core.PopScreen();
        ImGui::OpenPopup("Connection Error");
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return;
    }
    u_long mode = 1;
    int result = ioctlsocket(m_Socket, FIONBIO, &mode);
    if (result != NO_ERROR) {
        m_Core.PopScreen();
        ImGui::OpenPopup("Connection Error");
        closesocket(m_Socket);
        std::cerr << "Failed to set socket to non-blocking mode: " << WSAGetLastError() << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, m_IP, &serverAddr.sin_addr);

    result = connect(m_Socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR && (WSAGetLastError() != WSAEWOULDBLOCK)) {
        m_Core.PopScreen();
        ImGui::OpenPopup("Connection Error");
        closesocket(m_Socket);
        std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
        return;
    }
}

void ConnectingScreen::OnExit() {
    delete[] m_IP;
    m_IP = nullptr;
}