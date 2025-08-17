#include "UICore.hpp"

#include <iostream>
#include <stdexcept>

#include "Constants.hpp"
#include "StartScreen.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "MenuScreen.hpp"


static void glfw_error_callback(int error, const char *description) {
    throw std::runtime_error("GLFW Error " + std::to_string(error) + ": " + description);
}

void Core::Init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    this->m_ScaleFactor = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    this->m_Window = glfwCreateWindow(Constants::WINDOW_WIDTH * this->m_ScaleFactor,
                                      Constants::WINDOW_HEIGHT * this->m_ScaleFactor, "Management System", nullptr,
                                      nullptr);

    if (!this->m_Window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }


    glfwMakeContextCurrent(this->m_Window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    this->m_IO = &ImGui::GetIO();
    this->m_IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    this->m_IO->DisplaySize = ImVec2(Constants::WINDOW_WIDTH * this->m_ScaleFactor,
                                     Constants::WINDOW_HEIGHT * this->m_ScaleFactor);

    ImGui::StyleColorsDark();
    this->m_Style = &ImGui::GetStyle();
    this->m_Style->ScaleAllSizes(this->m_ScaleFactor);
    this->m_Style->FontScaleDpi = this->m_ScaleFactor;

    ImGui_ImplGlfw_InitForOpenGL(this->m_Window, true);
    ImGui_ImplOpenGL3_Init(Constants::GLSL_VERSION.c_str());

    // PushScreen<DemoScreen>(*this);
    //Push Screen Here
    PushScreen<MenuScreen>(*this);
    PushScreen<StartScreen>(*this);
}

void Core::Start() {
    assert(this->m_Window != nullptr);
    while (!glfwWindowShouldClose(this->m_Window) && !this->m_ScreenStack.empty()) {
        glfwPollEvents();

        if (glfwGetWindowAttrib(this->m_Window, GLFW_ICONIFIED) != 0) {
            // If the window is minimized, skip rendering
            ImGui_ImplGlfw_Sleep(Constants::SLEEPTIME);
            continue;
        }

        Update();
    }
}

void Core::Shutdown() {
    while (this->m_ScreenStack.empty() == false) {
        this->m_ScreenStack.back()->OnExit();
        this->m_ScreenStack.pop_back();
    }
    if (this->m_Window != nullptr) {
        return;
    }
    if (!glfwWindowShouldClose(this->m_Window)) {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(this->m_Window);
    glfwTerminate();
    this->m_Window = nullptr;
    this->m_IO = nullptr;
    this->m_Context = nullptr;
    this->m_Style = nullptr;
}

void Core::TryPopScreen() {
    // std::cout << "In TryPopScreen() ..." << std::endl;
    if (this->m_ShouldPop && !this->m_ScreenStack.empty()) {
        this->m_ShouldPop = false;
        this->m_ScreenStack.back()->OnExit();
        this->m_ScreenStack.pop_back();
        if (this->m_ShouldChangeState) {
            this->m_ShouldChangeState = false;
            PushScreen(std::move(this->m_ChangedScreen));
        }
    }
}

ImGuiIO &Core::GetIO() const {
    assert(this->m_IO != nullptr);
    return *this->m_IO;
}

ImGuiContext &Core::GetContext() const {
    assert(this->m_Context != nullptr);
    return *this->m_Context;
}

ImGuiStyle &Core::GetStyle() const {
    assert(this->m_Style != nullptr);
    return *this->m_Style;
}

GLFWwindow *Core::GetWindow() const {
    assert(this->m_Window != nullptr);
    return this->m_Window;
}

Screen &Core::GetCurrentScreen() const {
    return *m_ScreenStack.back();
}

void Core::PushScreen(std::unique_ptr<Screen> &&screen) {
    this->m_ScreenStack.emplace_back(std::move(screen));
    this->m_ScreenStack.back()->Init();
}

void Core::PopScreen() {
    this->m_ShouldPop = true;
}

void Core::Update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(this->m_IO->DisplaySize);

    const auto windowBorderSize = ImGui::GetStyle().WindowBorderSize;
    const auto windowRounding = ImGui::GetStyle().WindowRounding;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Content", nullptr, GetWindowFlags());

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, windowBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, windowRounding);

    Screen &currentScreen = this->GetCurrentScreen();
    currentScreen.Render(this->m_IO->DeltaTime);

    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(Constants::CLEAR_COLOR.Value.x,
                 Constants::CLEAR_COLOR.Value.y,
                 Constants::CLEAR_COLOR.Value.z,
                 Constants::CLEAR_COLOR.Value.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(this->m_Window);

    TryPopScreen();
}

ImGuiWindowFlags Core::GetWindowFlags() {
    return ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
}
