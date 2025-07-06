#pragma warning(disable : 4005)

#include <imgui.h>
#include <imgui_internal.h>

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#define WINDOWS_OS
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
#define LINUX_OS
#define UNIX_OS
#else
#define MAC_OS
#define UNIX_OS
#endif

#include <cstdio>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>
#include <iostream>

#ifdef WINDOWS_OS
#include <direct.h> // _chdir
#define SetCurrentDir _chdir
#endif

#ifdef UNIX_OS
#include <unistd.h> // chdir
#define SetCurrentDir chdir
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Test Engine
#include <imgui_test_engine/imgui_te_engine.h>
#include <imgui_test_engine/imgui_te_ui.h>
#include <imgui_test_engine/imgui_te_utils.h>
#include <imgui_test_engine/imgui_te_exporters.h>
#include <imgui_test_engine/thirdparty/Str/Str.h>
#include <shared/imgui_app.h>

#include <tests/DialogTests.h>

static void FindVideoEncoder(char* out, int out_len)
{
    IM_ASSERT(out != nullptr);
    IM_ASSERT(out_len > 0);
    Str64 encoder_path("tools/");           // Assume execution from root repository folder by default (linux/macos)
    if (!ImFileExist(encoder_path.c_str()))
        ImFileFindInParents("tools/", 3, &encoder_path);     // Assume execution from imgui_test_suite/$(Configuration) (visual studio)
    encoder_path.append("ffmpeg");                           // Look for ffmpeg executable in tools folder (windows/macos)
#if _WIN32
    encoder_path.append(".exe");
#else
    if (!ImFileExist(encoder_path.c_str()))
        encoder_path.set("/usr/bin/ffmpeg");                 // Use system version path (linux)
#endif
    if (ImFileExist(encoder_path.c_str()))
        ImStrncpy(out, encoder_path.c_str(), out_len);
    else
        *out = 0;
}

int main(int vArgc, char** vArgv) {
#ifdef _MSC_VER
    // active memory leak detector
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // set current dir where the app is
    const std::string& path{vArgv[0]};
    const auto pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
        SetCurrentDir(path.substr(0, pos).c_str());
    }

    // Setup application backend
    ImGuiApp* app = ImGuiApp_ImplDefault_Create();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
#ifdef IMGUI_HAS_VIEWPORT
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
#if IMGUI_VERSION_NUM >= 19004
    io.ConfigDebugIsDebuggerPresent = ImOsIsDebuggerPresent();
#endif

    // Setup application
    app->DpiAware = false;
    app->SrgbFramebuffer = false;
    app->ClearColor = ImVec4(0.120f, 0.120f, 0.120f, 1.000f);
    app->InitCreateWindow(app, "ImCode Test app", ImVec2(1600, 1000));
    app->InitBackends(app);

    // Setup test engine
    ImGuiTestEngine* engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(engine);
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Normal; // Default to slowest mode in this demo
    test_io.ScreenCaptureFunc = ImGuiApp_ScreenCaptureFunc;
    test_io.ScreenCaptureUserData = (void*)app;
    FindVideoEncoder(test_io.VideoCaptureEncoderPath, IM_ARRAYSIZE(test_io.VideoCaptureEncoderPath));
    ImStrncpy(test_io.VideoCaptureEncoderParams, R"(-hide_banner -loglevel error -r $FPS -f rawvideo -pix_fmt rgba -s $WIDTHx$HEIGHT -i - -threads 0 -y -preset ultrafast -pix_fmt yuv420p -crf 20 $OUTPUT)", IM_ARRAYSIZE(test_io.VideoCaptureEncoderParams));
    ImStrncpy(test_io.GifCaptureEncoderParams, R"(-hide_banner -loglevel error -r $FPS -f rawvideo -pix_fmt rgba -s $WIDTHx$HEIGHT -i - -threads 0 -y -filter_complex \"split=2 [a] [b]; [a] palettegen [pal]; [b] [pal] paletteuse\" $OUTPUT)", IM_ARRAYSIZE(test_io.GifCaptureEncoderParams));

    // Optional: save test output in junit-compatible XML format.
    test_io.ExportResultsFilename = "./igfd_visual_tests_results.xml";
    test_io.ExportResultsFormat = ImGuiTestEngineExportFormat_JUnitXml;

    // Start test engine
    ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());
    ImGuiTestEngine_InstallDefaultCrashHandler();

    // Register tests
    RegisterIGFDDialogTests(engine);

    // Main loop
    bool aborted = false;
    while (!aborted)
    {
        if (!aborted && !app->NewFrame(app))
            aborted = true;
        if (app->Quit)
            aborted = true;
        if (aborted && ImGuiTestEngine_TryAbortEngine(engine))
            break;

        ImGui::NewFrame();

        // Show windows
        ImGui::ShowDemoWindow();
        ImGuiTestEngine_ShowTestEngineWindows(engine, nullptr);

        // Render and swap
        app->Vsync = test_io.IsRequestingMaxAppSpeed ? false : true;
        ImGui::Render();
        app->Render(app);

        if (ImGuiTestEngine_IsUsingSimulatedInputs(engine) && !test_io.ConfigMouseDrawCursor && !test_io.IsCapturing) {
            ImGui::RenderMouseCursor(                                                          //
                ImGui::GetMousePos(), 1.2f, ImGui::GetMouseCursor(),                           //
                IM_COL32(255, 255, 120, 255), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 60));  // Custom yellow cursor
        }
        
        // Post-swap handler is REQUIRED in order to support screen capture
        ImGuiTestEngine_PostSwap(engine);
    }

    // Shutdown
    ImGuiTestEngine_Stop(engine);
    app->ShutdownBackends(app);
    app->ShutdownCloseWindow(app);
    ImGui::DestroyContext();

    // IMPORTANT: we need to destroy the Dear ImGui context BEFORE the test engine context, so .ini data may be saved.
    ImGuiTestEngine_DestroyContext(engine);

    app->Destroy(app);

    return 0;
}
