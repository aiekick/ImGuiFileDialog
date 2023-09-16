// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "imgui_internal.h"

#include "3rdparty/imgui/backends/imgui_impl_opengl3.h"
#include "3rdparty/imgui/backends/imgui_impl_glfw.h"
#include "ImGuiFileDialog.h"
#include "CustomFont.cpp"
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
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

static bool canValidateDialog = false;

inline void InfosPane(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue)  // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");

    ImGui::Text("Selected Filter : %s", vFilter);

    const char* userDatas = (const char*)vUserDatas;
    if (userDatas) ImGui::Text("User Datas : %s", userDatas);

    ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

    if (vCantContinue) *vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, const char* help, bool active, bool disabled) {
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    float w                 = CalcItemWidth();
    if (w == window->ItemWidthDefault) w = 0.0f;  // no push item width
    const ImGuiID id        = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    ImVec2 bb_size          = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
    bb_size.x               = ImMax(w, bb_size.x);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + bb_size);
    ItemSize(check_bb, style.FramePadding.y);

    if (!ItemAdd(check_bb, id)) return false;

    // check
    bool pressed = false;
    if (!disabled) {
        bool hovered, held;
        pressed = ButtonBehavior(check_bb, id, &hovered, &held);

        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
        if (active) {
            const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
        }
    }

    // circle shadow + bg
    if (style.FrameBorderSize > 0.0f) {
        window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (label_size.x > 0.0f) {
        RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
    }

    if (help && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", help);

    return pressed;
}

template <typename T>
inline bool RadioButtonLabeled_BitWize(const char* vLabel, const char* vHelp, T* vContainer, T vFlag,
                                       bool vOneOrZeroAtTime     = false,  // only one selected at a time
                                       bool vAlwaysOne           = true,   // radio behavior, always one selected
                                       T vFlagsToTakeIntoAccount = (T)0, bool vDisableSelection = false,
                                       ImFont* vLabelFont = nullptr)  // radio witl use only theses flags
{
    (void)vLabelFont;  // remove unused warnings

    bool selected  = (*vContainer) & vFlag;
    const bool res = RadioButtonLabeled(vLabel, vHelp, selected, vDisableSelection);
    if (res) {
        if (!selected) {
            if (vOneOrZeroAtTime) {
                if (vFlagsToTakeIntoAccount) {
                    if (vFlag & vFlagsToTakeIntoAccount) {
                        *vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount);  // remove these flags
                        *vContainer = (T)(*vContainer | vFlag);                     // add
                    }
                } else
                    *vContainer = vFlag;  // set
            } else {
                if (vFlagsToTakeIntoAccount) {
                    if (vFlag & vFlagsToTakeIntoAccount) {
                        *vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount);  // remove these flags
                        *vContainer = (T)(*vContainer | vFlag);                     // add
                    }
                } else
                    *vContainer = (T)(*vContainer | vFlag);  // add
            }
        } else {
            if (vOneOrZeroAtTime) {
                if (!vAlwaysOne) *vContainer = (T)(0);  // remove all
            } else
                *vContainer = (T)(*vContainer & ~vFlag);  // remove one
        }
    }
    return res;
}

class CustomDrawReadOnlyCheckBoxFileDialog : public ImGuiFileDialog {
private:
    bool m_ReadOnly = false;

public:
    static CustomDrawReadOnlyCheckBoxFileDialog* Instance(CustomDrawReadOnlyCheckBoxFileDialog* vCopy = nullptr, bool vForce = false) {
        static CustomDrawReadOnlyCheckBoxFileDialog _instance;
        static CustomDrawReadOnlyCheckBoxFileDialog* _instance_copy = nullptr;
        if (vCopy || vForce) {
            _instance_copy = vCopy;
        }
        if (_instance_copy) {
            return _instance_copy;
        }
        return &_instance;
    }

public:
    void OpenDialog(const std::string& vKey, const std::string& vTitle, const char* vFilters, const std::string& vPath, const std::string& vFileName, const int& vCountSelectionMax, IGFD::UserDatas vUserDatas, ImGuiFileDialogFlags vFlags) {
        m_ReadOnly = false;
        ImGuiFileDialog::OpenDialog(vKey, vTitle, vFilters, vPath, vFileName, vCountSelectionMax, vUserDatas, vFlags);
    }

    bool isReadOnly() {
        return m_ReadOnly;
    }

protected:
    bool prDrawFooter() override {
        auto& fdFile = prFileDialogInternal.puFileManager;

        float posY = ImGui::GetCursorPos().y;  // height of last bar calc
        ImGui::AlignTextToFramePadding();
        if (!fdFile.puDLGDirectoryMode)
            ImGui::Text("File Name :");
        else  // directory chooser
            ImGui::Text("Directory Path :");
        ImGui::SameLine();

        // Input file fields
        float width = ImGui::GetContentRegionAvail().x;
        if (!fdFile.puDLGDirectoryMode) {
            ImGuiContext& g = *GImGui;
            width -= 150.0f + g.Style.ItemSpacing.x;
        }

        ImGui::PushItemWidth(width);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_ReadOnlyFileNameField) {
            flags |= ImGuiInputTextFlags_ReadOnly;
        }
        if (ImGui::InputText("##FileName", fdFile.puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, flags)) {
            prFileDialogInternal.puIsOk = true;
        }
        if (ImGui::GetItemID() == ImGui::GetActiveID()) prFileDialogInternal.puFileInputIsActive = true;
        ImGui::PopItemWidth();

        // combobox of filters
        prFileDialogInternal.puFilterManager.DrawFilterComboBox(prFileDialogInternal);

        bool res = prDrawValidationButtons();

        ImGui::SameLine();

        if (ImGui::Checkbox("Read-Only", &m_ReadOnly)) {
            if (m_ReadOnly) {
                // remove confirm overwirte check since we are read only
                prFileDialogInternal.puDLGflags &= ~ImGuiFileDialogFlags_ConfirmOverwrite;
            } else {
                // add confirm overwirte since is what we want in our case
                prFileDialogInternal.puDLGflags |= ImGuiFileDialogFlags_ConfirmOverwrite;
            }
        }

        prFileDialogInternal.puFooterHeight = ImGui::GetCursorPosY() - posY;
        return res;
    }
};

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

        // Decide GL+GLSL versions
#if APPLE
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

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows
    io.FontAllowUserScaling = true;  // zoom wiht ctrl + mouse wheel

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiFileDialog fileDialog2;
    ImGuiFileDialog fileDialogEmbedded3;

    // an override for have read only checkbox
    static bool _IsFileReadOnly = false;
    CustomDrawReadOnlyCheckBoxFileDialog customFileDialog;

#ifdef USE_THUMBNAILS
    ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info) -> void {
        if (vThumbnail_Info && vThumbnail_Info->isReadyToUpload && vThumbnail_Info->textureFileDatas) {
            GLuint textureId = 0;
            glGenTextures(1, &textureId);
            vThumbnail_Info->textureID = (void*)(size_t)textureId;

            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)vThumbnail_Info->textureWidth, (GLsizei)vThumbnail_Info->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, vThumbnail_Info->textureFileDatas);
            glFinish();
            glBindTexture(GL_TEXTURE_2D, 0);

            delete[] vThumbnail_Info->textureFileDatas;
            vThumbnail_Info->textureFileDatas = nullptr;

            vThumbnail_Info->isReadyToUpload  = false;
            vThumbnail_Info->isReadyToDisplay = true;
        }
    });
    fileDialogEmbedded3.SetCreateThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info) -> void {
        if (vThumbnail_Info && vThumbnail_Info->isReadyToUpload && vThumbnail_Info->textureFileDatas) {
            GLuint textureId = 0;
            glGenTextures(1, &textureId);
            vThumbnail_Info->textureID = (void*)(size_t)textureId;

            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)vThumbnail_Info->textureWidth, (GLsizei)vThumbnail_Info->textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, vThumbnail_Info->textureFileDatas);
            glFinish();
            glBindTexture(GL_TEXTURE_2D, 0);

            delete[] vThumbnail_Info->textureFileDatas;
            vThumbnail_Info->textureFileDatas = nullptr;

            vThumbnail_Info->isReadyToUpload  = false;
            vThumbnail_Info->isReadyToDisplay = true;
        }
    });
    ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info) {
        if (vThumbnail_Info) {
            GLuint texID = (GLuint)(size_t)vThumbnail_Info->textureID;
            glDeleteTextures(1, &texID);
            glFinish();
        }
    });
    fileDialogEmbedded3.SetDestroyThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info) {
        if (vThumbnail_Info) {
            GLuint texID = (GLuint)(size_t)vThumbnail_Info->textureID;
            glDeleteTextures(1, &texID);
            glFinish();
        }
    });
#endif  // USE_THUMBNAILS

    ImGui::GetIO().Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = {ICON_MIN_IGFD, ICON_MAX_IGFD, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode  = true;
    icons_config.PixelSnapH = true;
    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);

    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color    = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // singleton acces
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "((Custom.+[.]h))", ImVec4(0.1f, 0.9f, 0.1f, 0.9f));  // use a regex
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);     // for all dirs
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(0.2f, 0.9f, 0.2f, 0.9f), ICON_IGFD_FILE);                              // for all link files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER);  // for all link dirs
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE);   // for all link files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);
    // set file style with a lambda function
    // return true is a file style was defined
    ImGuiFileDialog::Instance()->SetFileStyle([](const IGFD::FileInfos& vFileInfos, IGFD::FileStyle& vOutStyle) -> bool {
        if (!vFileInfos.fileNameExt.empty() && vFileInfos.fileNameExt[0] == '.') {
            vOutStyle = IGFD::FileStyle(ImVec4(0.0f, 0.9f, 0.9f, 1.0f), ICON_IGFD_REMOVE, nullptr);
            return true;
        }
        return false;
    });

    // just for show multi dialog instance behavior (here use for show directory query dialog)
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    fileDialog2.SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    fileDialog2.SetFileStyle(IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);

    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    fileDialogEmbedded3.SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);

    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "((Custom.+[.]h))", ImVec4(0.1f, 0.9f, 0.1f, 0.9f));  // use a regex
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);     // for all dirs
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(0.2f, 0.9f, 0.2f, 0.9f), ICON_IGFD_FILE);                              // for all link files
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER);  // for all link dirs
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE);   // for all link files
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);
    // set file style with a lambda function
    // return true is a file style was defined
    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->SetFileStyle([](const IGFD::FileInfos& vFile, IGFD::FileStyle& vOutStyle) -> bool {
        if (!vFile.fileNameExt.empty() && vFile.fileNameExt[0] == '.') {
            vOutStyle = IGFD::FileStyle(ImVec4(0.0f, 0.9f, 0.9f, 1.0f), ICON_IGFD_REMOVE);
            return true;
        }
        return false;
    });

    // c interface
    auto cfileDialog = IGFD_Create();
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC, nullptr);  // add an icon for the filter type
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]", nullptr);             // add an text for a filter type
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC, nullptr);
    IGFD_SetFileStyle(cfileDialog, IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK, nullptr);

#ifdef USE_BOOKMARK
    // load bookmarks
    std::ifstream docFile_1("bookmarks_1.conf", std::ios::in);
    if (docFile_1.is_open()) {
        std::stringstream strStream;
        strStream << docFile_1.rdbuf();  // read the file
        ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
        docFile_1.close();
    }

    std::ifstream docFile_2("bookmarks_2.conf", std::ios::in);
    if (docFile_2.is_open()) {
        std::stringstream strStream;
        strStream << docFile_2.rdbuf();  // read the file
        fileDialog2.DeserializeBookmarks(strStream.str());
        docFile_2.close();
    }

    // c interface
    std::ifstream docFile_c("bookmarks_c.conf", std::ios::in);
    if (docFile_c.is_open()) {
        std::stringstream strStream;
        strStream << docFile_c.rdbuf();  // read the file
        IGFD_DeserializeBookmarks(cfileDialog, strStream.str().c_str());
        docFile_c.close();
    }

    // add bookmark by code (why ? because we can :-) )
    ImGuiFileDialog::Instance()->AddBookmark("Current Dir", ".");
#endif

    static std::string filePathName;
    static std::string filePath;
    static std::string filter;
    static std::string userDatas;
    static std::vector<std::pair<std::string, std::string>> selection = {};

    static bool _UseWindowContraints  = true;
    static ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_Default;
    static IGFD_ResultMode resultMode = IGFD_ResultMode_AddIfNoFileExt;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        ImGui::Begin("imGuiFileDialog Demo");  // Create a window called "Hello, world!" and append into it.
        {
            ImGui::Text("imGuiFileDialog Demo %s : ", IMGUIFILEDIALOG_VERSION);
            ImGui::Separator();

            ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
            if (ImGui::CollapsingHeader("Settings")) {
#ifdef USE_EXPLORATION_BY_KEYS
                static float flashingAttenuationInSeconds = 1.0f;
                if (ImGui::Button("R##resetflashlifetime")) {
                    flashingAttenuationInSeconds = 1.0f;
                    ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                    fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                    // c interface
                    IGFD_SetFlashingAttenuationInSeconds(cfileDialog, flashingAttenuationInSeconds);
                }
                ImGui::SameLine();
                ImGui::PushItemWidth(200);
                if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f)) {
                    ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                    fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                    // c interface
                    IGFD_SetFlashingAttenuationInSeconds(cfileDialog, flashingAttenuationInSeconds);
                }
                ImGui::PopItemWidth();
#endif
                ImGui::Separator();
                ImGui::Checkbox("Use file dialog constraint", &_UseWindowContraints);
                ImGui::Text("Constraints is used here for define min/max file dialog size");
                ImGui::Separator();

                ImGui::Text("ImGuiFileDialog Flags : ");
                ImGui::Indent();
                {
                    ImGui::Text("Commons :");
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Modal", "Open dialog in modal mode", &flags, ImGuiFileDialogFlags_Modal);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Overwrite", "Overwrite verification before dialog closing", &flags, ImGuiFileDialogFlags_ConfirmOverwrite);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Hidden Files", "Hide Hidden Files", &flags, ImGuiFileDialogFlags_DontShowHiddenFiles);

                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable Directory Creation", "Disable Directory Creation button in dialog", &flags, ImGuiFileDialogFlags_DisableCreateDirectoryButton);
#ifdef USE_THUMBNAILS
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable thumbnails mode", "Disable thumbnails display in dialo", &flags, ImGuiFileDialogFlags_DisableThumbnailMode);
#endif  // USE_THUMBNAILS
#ifdef USE_BOOKMARK
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable bookmark mode", "Disable bookmark display in dialo", &flags, ImGuiFileDialogFlags_DisableBookmarkMode);
#endif  // USE_BOOKMARK

                    ImGui::Text("Hide Column by default : (saved in imgui.ini, \n\tso defined when the imgui.ini is not existing)");
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Type", "Hide Column file type by default", &flags, ImGuiFileDialogFlags_HideColumnType);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Size", "Hide Column file Size by default", &flags, ImGuiFileDialogFlags_HideColumnSize);
                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Date", "Hide Column file Date by default", &flags, ImGuiFileDialogFlags_HideColumnDate);

                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Case Insensitive Extentions", "will not take into account the case of file extentions", &flags, ImGuiFileDialogFlags_CaseInsensitiveExtention);

                    ImGui::SameLine();
                    RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable quick path selection", "Disable the quick path selection", &flags, ImGuiFileDialogFlags_DisableQuickPathSelection);
                    
                    ImGui::Separator();
                    ImGui::Text("Result Modes : for GetFilePathName and GetSelection");
                    
                    if (RadioButtonLabeled("Add If No File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt;
                    }
                    ImGui::SameLine();
                    if (RadioButtonLabeled("Overwrite File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt;
                    }
                    ImGui::SameLine();
                    if (RadioButtonLabeled("Keep Input File", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile, false)) {
                        resultMode = IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile;
                    }
                }
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Singleton acces :")) {
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog")) {
                    const char* filters = ".*,.cpp,.h,.hpp";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters")) {
                    const char* filters =
                        "All files{.*},Frames Format 1(.001,.NNN){(([.][0-9]{3}))},Frames Format 2(nnn.png){(([0-9]*.png))},Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with filter of type regex ((Custom.+[.]h))")) {
                    // the regex for being recognized at regex need to be between ( and )
                    const char* filters = "Regex Custom*.h{((Custom.+[.]h))}";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items")) {
                    const char* filters = ".*,.cpp,.h,.hpp";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 5, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection")) {
                    const char* filters = ".*,.cpp,.h,.hpp";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with last file path name")) {
                    const char* filters = ".*,.cpp,.h,.hpp";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, filePathName, 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open All file types with filter .*")) {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", "", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer filter")) {
                    const char* filters = ".a.b";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer .*.*")) {
                    const char* filters = ".*.*";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer .vcxproj.*")) {
                    const char* filters = ".vcxproj.*";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with regex (([.]vcx))")) {
                    const char* filters = "(([.]vcx))";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open All file types with a multilayer collectionfilter")) {
                    const char* filters = "multi layers{.filters, .a.b }";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane")) {
                    const char* filters = "C++ File (*.cpp){.cpp}";
                    ImGuiFileDialog::Instance()->OpenDialogWithPane("ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, ".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 350, 1,
                                                            IGFDUserDatas("SaveFile"), flags);
                }
                if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with Confirm Dialog For Overwrite File if exist")) {
                    const char* filters = "C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, ".", "", 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
                }
            }

            if (ImGui::CollapsingHeader("Open Directories :")) {
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog")) {
                    // let filters be null for open directory chooser
                    fileDialog2.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", 1, nullptr, flags);
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items")) {
                    // set filters be null for open directory chooser
                    fileDialog2.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", "", 5, nullptr, flags);
                }
            }

            if (ImGui::CollapsingHeader("Draw Override of the FileDialog for have a read only checkbox")) {
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open A Draww Override FileDialog with a read only btn")) {
                    const char* filters =
                        "All files{.*},Frames Format 1(.001,.NNN){(([.][0-9]{3}))},Frames Format 2(nnn.png){(([0-9]*.png))},Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
                    CustomDrawReadOnlyCheckBoxFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
                }
                ImGui::Text("Is File Read only ?? : %s", _IsFileReadOnly ? "yes" : "false");

                /////////////////////////////////////////////////////////////////
                // C Interface
                /////////////////////////////////////////////////////////////////
            }

            if (ImGui::CollapsingHeader("C API instance demo")) {
                if (ImGui::Button("C " ICON_IGFD_SAVE " Save File Dialog with a custom pane")) {
                    const char* filters = "C++ File (*.cpp){.cpp}";
                    IGFD_OpenDialogWithPane(cfileDialog, "ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, ".", "", &InfosPane, 350, 1, (void*)("SaveFile"), flags);
                }
            }
            /////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////

            if (ImGui::CollapsingHeader("Embedded Dialog demo :")) {
                fileDialogEmbedded3.OpenDialog("embedded", "Select File", ".*", "", -1, nullptr,
                                               ImGuiFileDialogFlags_NoDialog |
#ifdef USE_BOOKMARK
                                                   ImGuiFileDialogFlags_DisableBookmarkMode |
#endif  // USE_BOOKMARK
                                                   ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_ReadOnlyFileNameField);
                // to note, when embedded only the vMinSize do nothing, only the vMaxSize can size the dialog frame
                if (fileDialogEmbedded3.Display("embedded", ImGuiWindowFlags_NoCollapse, ImVec2(0, 0), ImVec2(0, 350))) {
                    if (fileDialogEmbedded3.IsOk()) {
                        filePathName = ImGuiFileDialog::Instance()->GetFilePathName(resultMode);
                        filePath     = ImGuiFileDialog::Instance()->GetCurrentPath();
                        filter       = ImGuiFileDialog::Instance()->GetCurrentFilter();
                        // here convert from string because a string was passed as a userDatas, but it can be what you want
                        if (ImGuiFileDialog::Instance()->GetUserDatas()) userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
                        auto sel = ImGuiFileDialog::Instance()->GetSelection(resultMode);  // multiselection
                        selection.clear();
                        for (auto s : sel) {
                            selection.emplace_back(s.first, s.second);
                        }
                        // action
                    }
                    fileDialogEmbedded3.Close();
                }
            }

            if (ImGui::CollapsingHeader("ImGuiFileDialog Return's :")) {
                ImGui::Text("GetFilePathName() : %s", filePathName.c_str());
                ImGui::Text("GetFilePath() : %s", filePath.c_str());
                ImGui::Text("GetCurrentFilter() : %s", filter.c_str());
                ImGui::Text("GetUserDatas() (was a std::string in this sample) : %s", userDatas.c_str());
                ImGui::Text("GetSelection() : ");
                ImGui::Indent();
                {
                    static int selected = false;
                    if (ImGui::BeginTable("##GetSelection", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
                        ImGui::TableSetupScrollFreeze(0, 1);  // Make top row always visible
                        ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch, -1, 0);
                        ImGui::TableSetupColumn("File Path name", ImGuiTableColumnFlags_WidthFixed, -1, 1);
                        ImGui::TableHeadersRow();

                        ImGuiListClipper clipper;
                        clipper.Begin((int)selection.size(), ImGui::GetTextLineHeightWithSpacing());
                        while (clipper.Step()) {
                            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                                const auto& sel = selection[i];
                                ImGui::TableNextRow();
                                if (ImGui::TableSetColumnIndex(0))  // first column
                                {
                                    ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
                                    selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                                    if (ImGui::Selectable(sel.first.c_str(), i == selected, selectableFlags)) selected = i;
                                }
                                if (ImGui::TableSetColumnIndex(1))  // second column
                                {
                                    ImGui::Text("%s", sel.second.c_str());
                                }
                            }
                        }
                        clipper.End();

                        ImGui::EndTable();
                    }
                }
                ImGui::Unindent();
            }

            /////////////////////////////////////////////////////////////////
            //// DISPLAY ////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////

            ImVec2 minSize = ImVec2(0, 0);
            ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

            if (_UseWindowContraints) {
                maxSize = ImVec2((float)display_w, (float)display_h) * 0.7f;
                minSize = maxSize * 0.25f;
            }

            // you can define your flags and min/max window size (theses three settings ae defined by default :
            // flags => ImGuiWindowFlags_NoCollapse
            // minSize => 0,0
            // maxSize => FLT_MAX, FLT_MAX (defined is float.h)

            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    filePathName = ImGuiFileDialog::Instance()->GetFilePathName(resultMode);
                    filePath     = ImGuiFileDialog::Instance()->GetCurrentPath();
                    filter       = ImGuiFileDialog::Instance()->GetCurrentFilter();
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    if (ImGuiFileDialog::Instance()->GetUserDatas()) userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
                    auto sel = ImGuiFileDialog::Instance()->GetSelection(resultMode);  // multiselection
                    selection.clear();
                    for (auto s : sel) {
                        selection.emplace_back(s.first, s.second);
                    }
                    // action
                }
                ImGuiFileDialog::Instance()->Close();
            }

            if (fileDialog2.Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (fileDialog2.IsOk()) {
                    filePathName = fileDialog2.GetFilePathName(resultMode);
                    filePath     = fileDialog2.GetCurrentPath();
                    filter       = fileDialog2.GetCurrentFilter();
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    if (fileDialog2.GetUserDatas()) userDatas = std::string((const char*)fileDialog2.GetUserDatas());
                    auto sel = fileDialog2.GetSelection(resultMode);  // multiselection
                    selection.clear();
                    for (auto s : sel) {
                        selection.emplace_back(s.first, s.second);
                    }
                    // action
                }
                fileDialog2.Close();
            }

            if (CustomDrawReadOnlyCheckBoxFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (CustomDrawReadOnlyCheckBoxFileDialog::Instance()->IsOk()) {
                    _IsFileReadOnly = CustomDrawReadOnlyCheckBoxFileDialog::Instance()->isReadOnly();
                }
                CustomDrawReadOnlyCheckBoxFileDialog::Instance()->Close();
            }

            /////////////////////////////////////////////////////////////////
            // C Interface
            /////////////////////////////////////////////////////////////////
            if (IGFD_DisplayDialog(cfileDialog, "ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
                if (IGFD_IsOk(cfileDialog)) {
                    char* cfilePathName = IGFD_GetFilePathName(cfileDialog, IGFD_ResultMode_AddIfNoFileExt);
                    if (cfilePathName) filePathName = cfilePathName;
                    char* cfilePath = IGFD_GetCurrentPath(cfileDialog);
                    if (cfilePath) filePath = cfilePath;
                    char* cfilter = IGFD_GetCurrentFilter(cfileDialog);
                    if (cfilter) filter = cfilter;
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    void* cdatas = IGFD_GetUserDatas(cfileDialog);
                    if (cdatas) userDatas = (const char*)cdatas;
                    IGFD_Selection csel = IGFD_GetSelection(cfileDialog, IGFD_ResultMode_KeepInputFile);  // multiselection

                    selection.clear();
                    for (size_t i = 0; i < csel.count; i++) {
                        std::string _fileName     = csel.table[i].fileName;
                        std::string _filePathName = csel.table[i].filePathName;
                        selection.emplace_back(_fileName, _filePathName);
                    }

                    // destroy
                    free(cfilePathName);
                    free(cfilePath);
                    free(cfilter);
                    IGFD_Selection_DestroyContent(&csel);
                }
                IGFD_CloseDialog(cfileDialog);
            }
        }
        ImGui::End();

        // Cpu Zone : prepare
        ImGui::Render();

        // GPU Zone : Rendering
        glfwMakeContextCurrent(window);

#ifdef USE_THUMBNAILS
        ImGuiFileDialog::Instance()->ManageGPUThumbnails();
        fileDialogEmbedded3.ManageGPUThumbnails();
#endif

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        glfwSwapBuffers(window);
    }

#ifdef USE_BOOKMARK
    // remove bookmark
    ImGuiFileDialog::Instance()->RemoveBookmark("Current Dir");

    // save bookmarks dialog 1
    std::ofstream configFileWriter_1("bookmarks_1.conf", std::ios::out);
    if (!configFileWriter_1.bad()) {
        configFileWriter_1 << ImGuiFileDialog::Instance()->SerializeBookmarks();
        configFileWriter_1.close();
    }
    // save bookmarks dialog 2
    std::ofstream configFileWriter_2("bookmarks_2.conf", std::ios::out);
    if (!configFileWriter_2.bad()) {
        configFileWriter_2 << fileDialog2.SerializeBookmarks();
        configFileWriter_2.close();
    }
    // save bookmarks dialog c interface
    std::ofstream configFileWriter_c("bookmarks_c.conf", std::ios::out);
    if (!configFileWriter_c.bad()) {
        char* s = IGFD_SerializeBookmarks(cfileDialog, true);
        if (s) {
            configFileWriter_c << std::string(s);
            configFileWriter_c.close();
        }
    }
#endif

    // c interface
    IGFD_Destroy(cfileDialog);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
