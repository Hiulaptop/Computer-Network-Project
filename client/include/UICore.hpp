#pragma once
#include <memory>
#include <vector>
#include <cstdint>

#include "imgui.h"
#include "GLFW/glfw3.h"
#include "Screen.hpp"
class Core {
private:
    float m_ScaleFactor = 0;
    ImGuiIO *m_IO = nullptr;
    ImGuiContext *m_Context = nullptr;
    ImGuiStyle *m_Style = nullptr;
    GLFWwindow *m_Window = nullptr;

    bool m_ShouldPop = false;

    void Update();

    static ImGuiWindowFlags GetWindowFlags();

    std::vector<std::unique_ptr<Screen>> m_ScreenStack;

public:
    ~Core() = default;

    void Init();

    void Start();

    void Shutdown();

    void TryPopScreen();

    ImGuiIO &GetIO() const;

    ImGuiContext &GetContext() const;

    ImGuiStyle &GetStyle() const;

    GLFWwindow *GetWindow() const;

    Screen &GetCurrentScreen() const;

    void PushScreen(std::unique_ptr<Screen> &&screen);

    template<typename T, typename... Args>
    void PushScreen(Args &&... args) {
        static_assert(std::is_base_of_v<Screen, T>, "T must be derived from Screen");
        this->PushScreen(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void PopScreen();
};
