// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#include "imgui_internal.h"

#include <src/gui/DemoDialog.h>

#include "3rdparty/imgui/backends/imgui_impl_opengl3.h"
#include "3rdparty/imgui/backends/imgui_impl_glfw.h"

#include <cstdio>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* pWindow{nullptr};
DemoDialog demoDialog;
ImVec4 viewportRect;

static void sUpdate(void*) {
    int display_w{};
    int display_h{};
    ImGuiIO& io = ImGui::GetIO();

#ifndef __EMSCRIPTEN__
    glfwPollEvents();
#endif

    glfwGetFramebufferSize(pWindow, &display_w, &display_h);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#ifdef IMGUI_HAS_VIEWPORT
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        const auto viewport = ImGui::GetMainViewport();
        if (viewport) {
            const auto pos  = viewport->WorkPos;
            const auto size = viewport->WorkSize;
            viewportRect.x  = pos.x;
            viewportRect.y  = pos.y;
            viewportRect.z  = size.x;
            viewportRect.w  = size.y;
        }
    } else {
        viewportRect.x = 0;
        viewportRect.y = 0;
    }
#endif

    try {
        demoDialog.display(viewportRect);
    } catch (std::exception& ex) {
        std::cout << "exception catched with message : " << ex.what() << std::endl;
        assert(0);
    }

    // Cpu Zone : prepare
    ImGui::Render();

    // GPU Zone : Rendering
    glfwMakeContextCurrent(pWindow);

    // manage thumbnails
    demoDialog.manageGPU();

    glViewport(0, 0, display_w, display_h);
    glClearColor(demoDialog.clear_color.x, demoDialog.clear_color.y, demoDialog.clear_color.z, demoDialog.clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifndef __EMSCRIPTEN__
#ifdef IMGUI_HAS_VIEWPORT
    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
#endif
#endif

    glfwSwapBuffers(pWindow);
}

int main(int, char**) {
#ifdef _MSC_VER
    // active memory leak detector
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    auto loc = std::setlocale(LC_ALL, ".UTF8");
    if (!loc) {
        printf("setlocale fail to apply with this compiler. it seems the unicode will be NOK\n");
    }

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

#ifdef __EMSCRIPTEN__
    // GL ES 3.0 + GLSL 130
    const char* glsl_version = "#version 300 es";
#else
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
#endif

    // Create window with graphics context
    pWindow = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (pWindow == nullptr) return 1;
    glfwMakeContextCurrent(pWindow);
    glfwSwapInterval(1);  // Enable vsync

#ifndef __EMSCRIPTEN__
// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false;  // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
#endif

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
#ifndef __EMSCRIPTEN__
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows
#endif
    io.ConfigViewportsNoDecoration = false;  // no decoration, for avoid a resuize bug
    io.FontAllowUserScaling        = true;   // zoom wiht ctrl + mouse wheel

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    demoDialog.init(15.0f);

    ImVec4 viewportRect;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(sUpdate, nullptr, 0, true);
#else
    while (!glfwWindowShouldClose(pWindow)) {
        sUpdate(nullptr);
    }
#endif

    demoDialog.unit();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(pWindow);
    glfwTerminate();

    return 0;
}
