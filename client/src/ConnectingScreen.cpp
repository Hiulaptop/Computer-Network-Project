#include "ConnectingScreen.hpp"

#include <complex.h>
#include <iostream>

#include "MainMenu.hpp"
#include "UICore.hpp"

char* ConnectingScreen::m_IP = nullptr;
std::atomic_bool ConnectingScreen::m_isConnected(false);
std::atomic_bool ConnectingScreen::m_isError(false);

void ConnectingScreen::Render(float DT) {
    if (m_totalTime > 5.f || m_isError.load()) {
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
    if (m_isConnected.load()) {
        m_Core.ChangeScreen<MainMenu>(m_Core, m_IP);
    }
}

void ConnectingScreen::Init() {
    CreateThread(
        nullptr,
        0,
        Connecting,
        nullptr,
        0,
        nullptr
    );
    m_totalTime = 0;
    m_isError = false;
    m_isConnected = false;
}

void ConnectingScreen::OnExit() {
    delete[] m_IP;
    m_IP = nullptr;
}

DWORD ConnectingScreen::Connecting(LPVOID lpParam) {
    SOCKET m_connectedSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_connectedSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        m_isError.store(true);
        return 1;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Replace with your server port
    inet_pton(AF_INET, m_IP, &serverAddr.sin_addr);
    if (connect(m_connectedSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
        closesocket(m_connectedSocket);
        m_connectedSocket = INVALID_SOCKET;
        m_isError.store(true);
        return 1;
    }
    PacketHeader header{};
    header.request_key = 5;
    header.request_id = 0;
    header.packet_size = sizeof(PacketHeader);
    header.request_type = 0;
    send(m_connectedSocket, reinterpret_cast<const char*>(&header), sizeof(header), 0);
    int bytesReceived = 0;
    ResponseHeader responseHeader{};
    bytesReceived = recv(m_connectedSocket, reinterpret_cast<char*>(&responseHeader), sizeof(responseHeader), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Failed to receive response header: " << WSAGetLastError() << std::endl;
        closesocket(m_connectedSocket);
        m_connectedSocket = INVALID_SOCKET;
        m_isError.store(true);
        return 1;
    }
    if (responseHeader.statusCode != 0x01) {
        std::cerr << "Failed to connect to server, status code: " << responseHeader.statusCode << std::endl;
        closesocket(m_connectedSocket);
        m_connectedSocket = INVALID_SOCKET;
        m_isError.store(true);
        return 1;
    }
    std::cout << "Successfully connected to server." << std::endl;
    m_isConnected.store(true);
    return 0;
}
