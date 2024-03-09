#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <DemoDialog.h>

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiFileDialog.h"
#include "CustomFont.cpp"
#include "Roboto_Medium.cpp"
//#include "HanyISentyPagodaRegular.cpp"
#include <cstdio>
#include <sstream>
#include <fstream>
#include <clocale>
#include <string>

#include <ImWidgets.h>

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#define stat _stat
#include <ShlObj.h>  // for get known folders
#else                // UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif  // __WIN32__

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>  // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>  // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_ES3)
#include <GLES3/gl3.h>          // Use GL ES 3
#endif

static bool canValidateDialog = false;

static std::string filePathName;
static std::string filePath;
static std::string filter;
static std::string userDatas;
static std::vector<std::pair<std::string, std::string> > selection = {};

static bool UseWindowContraints   = true;
static ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_Default;
static IGFD_ResultMode resultMode = IGFD_ResultMode_AddIfNoFileExt;

// an override for have read only checkbox
static bool IsFileReadOnly = false;
static IGFD::FileManager fileManager;

inline void InfosPane(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue)  // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");

    ImGui::Text("Selected Filter : %s", vFilter);

    const char* userDatas = (const char*)vUserDatas;
    if (userDatas) ImGui::Text("User Datas : %s", userDatas);

    ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

    if (vCantContinue) *vCantContinue = canValidateDialog;
}

void ApplyOrangeBlueTheme() {
    ImGuiStyle style;
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.26f, 0.28f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.21f, 0.29f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.71f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.18f, 0.20f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.23f, 0.25f, 0.26f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.30f, 0.33f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.21f, 0.29f, 0.36f, 0.89f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.13f, 0.52f, 0.94f, 0.45f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.13f, 0.71f, 1.00f, 0.89f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.24f, 0.78f, 0.78f, 0.31f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(1.00f, 0.60f, 0.00f, 0.80f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(1.00f, 0.48f, 0.00f, 0.80f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.13f, 0.52f, 0.94f, 0.66f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.13f, 0.52f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.13f, 0.52f, 0.94f, 0.59f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.35f, 0.58f, 0.59f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab]                   = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.13f, 0.52f, 0.94f, 0.95f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg]            = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.Colors[ImGuiCol_WindowBg].w   = 1.00f;
    style.Colors[ImGuiCol_ChildBg].w    = 0.00f;
    style.Colors[ImGuiCol_MenuBarBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Main
    style.WindowPadding     = ImVec2(4.00f, 4.00f);
    style.FramePadding      = ImVec2(4.00f, 4.00f);
    style.ItemSpacing       = ImVec2(4.00f, 4.00f);
    style.ItemInnerSpacing  = ImVec2(4.00f, 4.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing     = 8.00f;
    style.ScrollbarSize     = 10.00f;
    style.GrabMinSize       = 8.00f;

    // Borders
    style.WindowBorderSize = 0.00f;
    style.ChildBorderSize  = 0.00f;
    style.PopupBorderSize  = 1.00f;
    style.FrameBorderSize  = 0.00f;
    style.TabBorderSize    = 0.00f;

    // Rounding
    style.WindowRounding    = 2.00f;
    style.ChildRounding     = 2.00f;
    style.FrameRounding     = 2.00f;
    style.PopupRounding     = 2.00f;
    style.ScrollbarRounding = 2.00f;
    style.GrabRounding      = 2.00f;
    style.TabRounding       = 2.00f;

    // Alignment
    style.WindowTitleAlign         = ImVec2(0.50f, 0.50f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition      = ImGuiDir_Right;
    style.ButtonTextAlign          = ImVec2(0.50f, 0.50f);
    style.SelectableTextAlign      = ImVec2(0.00f, 0.50f);

    // Safe Area Padding
    style.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

    ImGui::GetStyle() = style;
}

std::string toStr(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char TempBuffer[3072 + 1];  // 3072 = 1024 * 3
    const int w = vsnprintf(TempBuffer, 3072, fmt, args);
    va_end(args);
    if (w) {
        return std::string(TempBuffer, (size_t)w);
    }
    return std::string();
}

CustomDrawReadOnlyCheckBoxFileDialog* CustomDrawReadOnlyCheckBoxFileDialog::Instance(CustomDrawReadOnlyCheckBoxFileDialog* vCopy , bool vForce ) {
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

void CustomDrawReadOnlyCheckBoxFileDialog::OpenDialog(const std::string& vKey, const std::string& vTitle, const char* vFilters, const IGFD::FileDialogConfig& vConfig) {
    m_ReadOnly = false;
    ImGuiFileDialog::OpenDialog(vKey, vTitle, vFilters, vConfig);
}

bool CustomDrawReadOnlyCheckBoxFileDialog::isReadOnly() const {
    return m_ReadOnly;
}

bool CustomDrawReadOnlyCheckBoxFileDialog::m_DrawFooter() {
    auto& fdFile = m_FileDialogInternal.fileManager;

    float posY = ImGui::GetCursorPos().y;  // height of last bar calc
    ImGui::AlignTextToFramePadding();
    if (!fdFile.dLGDirectoryMode)
        ImGui::Text("File Name :");
    else  // directory chooser
        ImGui::Text("Directory Path :");
    ImGui::SameLine();

    // Input file fields
    float width = ImGui::GetContentRegionAvail().x;
    if (!fdFile.dLGDirectoryMode) {
        ImGuiContext& g = *GImGui;
        width -= 150.0f + g.Style.ItemSpacing.x;
    }

    ImGui::PushItemWidth(width);
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (m_FileDialogInternal.getDialogConfig().flags & ImGuiFileDialogFlags_ReadOnlyFileNameField) {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }
    if (ImGui::InputText("##FileName", fdFile.fileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, flags)) {
        m_FileDialogInternal.isOk = true;
    }
    if (ImGui::GetItemID() == ImGui::GetActiveID()) m_FileDialogInternal.fileInputIsActive = true;
    ImGui::PopItemWidth();

    // combobox of filters
    m_FileDialogInternal.filterManager.DrawFilterComboBox(m_FileDialogInternal);

    bool res = m_DrawValidationButtons();

    ImGui::SameLine();

    if (ImGui::Checkbox("Read-Only", &m_ReadOnly)) {
        if (m_ReadOnly) {
            // remove confirm overwirte check since we are read only
            m_FileDialogInternal.getDialogConfigRef().flags &= ~ImGuiFileDialogFlags_ConfirmOverwrite;
        } else {
            // add confirm overwirte since is what we want in our case
            m_FileDialogInternal.getDialogConfigRef().flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
        }
    }

    m_FileDialogInternal.footerHeight = ImGui::GetCursorPosY() - posY;
    return res;
}

void DemoDialog::init(const float& vFontSize) {
    ImGui::CustomStyle::Init();
    ImGui::SetPUSHID(4577);

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    ApplyOrangeBlueTheme();

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
            auto texID = (GLuint)(size_t)vThumbnail_Info->textureID;
            glDeleteTextures(1, &texID);
            glFinish();
        }
    });
    fileDialogEmbedded3.SetDestroyThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info) {
        if (vThumbnail_Info) {
            auto texID = (GLuint)(size_t)vThumbnail_Info->textureID;
            glDeleteTextures(1, &texID);
            glFinish();
        }
    });
#endif  // USE_THUMBNAILS

    // Fonts
    {
        ImFontConfig icons_config_0;
        icons_config_0.GlyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
        //ImGui::GetIO().Fonts->AddFontDefault(&icons_config_0);
    }
    {
        ImFontConfig icons_config_1;
        icons_config_1.GlyphRanges = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
        //ImGui::GetIO().Fonts->AddFontFromFileTTF("NotoSansSCRegular.ttf", vFontSize, &icons_config_1);
    }
    {  // Boboto Font
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, vFontSize);
    }
    {  // glyphs
        static const ImWchar icons_ranges_3[] = {ICON_MIN_IGFD, ICON_MAX_IGFD, 0};
        ImFontConfig icons_config_3;
        icons_config_3.MergeMode  = true;
        icons_config_3.PixelSnapH = true;
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, vFontSize, &icons_config_3, icons_ranges_3);
    }

    // Our state

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
            vOutStyle.color = ImVec4(0.0f, 0.9f, 0.9f, 1.0f);
            vOutStyle.icon  = ICON_IGFD_REMOVE;
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
            vOutStyle.color = ImVec4(0.0f, 0.9f, 0.9f, 1.0f);
            vOutStyle.icon  = ICON_IGFD_REMOVE;
            return true;
        }
        return false;
    });

    // c interface
    cFileDialogPtr = IGFD_Create();
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f), "", nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC, nullptr);  // add an icon for the filter type
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]", nullptr);             // add an text for a filter type
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC, nullptr);
    IGFD_SetFileStyle(cFileDialogPtr, IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK, nullptr);

#ifdef USE_PLACES_FEATURE
    // load place
    std::ifstream docFile_1("places_1.conf", std::ios::in);
    if (docFile_1.is_open()) {
        std::stringstream strStream;
        strStream << docFile_1.rdbuf();  // read the file
        ImGuiFileDialog::Instance()->DeserializePlaces(strStream.str());
        docFile_1.close();
    }

    std::ifstream docFile_2("places_2.conf", std::ios::in);
    if (docFile_2.is_open()) {
        std::stringstream strStream;
        strStream << docFile_2.rdbuf();  // read the file
        fileDialog2.DeserializePlaces(strStream.str());
        docFile_2.close();
    }

    // c interface
    std::ifstream docFile_c("places_c.conf", std::ios::in);
    if (docFile_c.is_open()) {
        std::stringstream strStream;
        strStream << docFile_c.rdbuf();  // read the file
        IGFD_DeserializePlaces(cFileDialogPtr, strStream.str().c_str());
        docFile_c.close();
    }

    // add places :
    const char* group_name = ICON_IGFD_SHORTCUTS " Places";
    ImGuiFileDialog::Instance()->AddPlacesGroup(group_name, 1, false);
    IGFD_AddPlacesGroup(cFileDialogPtr, group_name, 1, false);

    // Places :
    auto places_ptr = ImGuiFileDialog::Instance()->GetPlacesGroupPtr(group_name);
    if (places_ptr != nullptr) {
#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#define addKnownFolderAsPlace(knownFolder, folderLabel, folderIcon)                                     \
    {                                                                                                   \
        PWSTR path = NULL;                                                                              \
        HRESULT hr = SHGetKnownFolderPath(knownFolder, 0, NULL, &path);                                 \
        if (SUCCEEDED(hr)) {                                                                            \
            IGFD::FileStyle style;                                                                      \
            style.icon      = folderIcon;                                                               \
            auto place_path = IGFD::Utils::UTF8Encode(path);                                            \
            places_ptr->AddPlace(folderLabel, place_path, false, style);                                \
            IGFD_AddPlace(cFileDialogPtr, group_name, folderLabel, place_path.c_str(), false, folderIcon); \
        }                                                                                               \
        CoTaskMemFree(path);                                                                            \
    }
        addKnownFolderAsPlace(FOLDERID_Desktop, "Desktop", ICON_IGFD_DESKTOP);
        addKnownFolderAsPlace(FOLDERID_Startup, "Startup", ICON_IGFD_HOME);
        places_ptr->AddPlaceSeparator(3.0f);  // add a separator
        addKnownFolderAsPlace(FOLDERID_Downloads, "Downloads", ICON_IGFD_DOWNLOADS);
        addKnownFolderAsPlace(FOLDERID_Pictures, "Pictures", ICON_IGFD_PICTURE);
        addKnownFolderAsPlace(FOLDERID_Music, "Music", ICON_IGFD_MUSIC);
        addKnownFolderAsPlace(FOLDERID_Videos, "Videos", ICON_IGFD_FILM);
#undef addKnownFolderAsPlace
#else
#endif
        places_ptr = nullptr;
    }

    // add place by code (why ? because we can :-) )
    // ImGuiFileDialog->
    // todo : do the code
    // ImGuiFileDialog::Instance()->AddPlace("Current Dir", ".");
#endif  // USE_PLACES_FEATURE
}

void DemoDialog::display(const int32_t& vDisplayWidth, const int32_t& vDisplayHeight) {
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    ImGui::Begin("ImGuiFileDialog Demo");  // Create a window called "Hello, world!" and append into it.
    {
        ImGui::Text("ImGuiFileDialog Demo : %s", IMGUIFILEDIALOG_VERSION);
        ImGui::Text("FileSystem Interface : %s", fileManager.GetFileSystemName().c_str());

        ImGui::Separator();

        ImGui::ColorEdit3("clear color", (float*)&clear_color);  // Edit 3 floats representing a color
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Checkbox("Demo Window", &show_demo_window);  // Edit bools storing our window open/close state
        if (ImGui::CollapsingHeader("Settings")) {
#ifdef USE_EXPLORATION_BY_KEYS
            static float flashingAttenuationInSeconds = 1.0f;
            if (ImGui::ContrastedButton("R##resetflashlifetime")) {
                flashingAttenuationInSeconds = 1.0f;
                ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                // c interface
                IGFD_SetFlashingAttenuationInSeconds(cFileDialogPtr, flashingAttenuationInSeconds);
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(200);
            if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f)) {
                ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
                fileDialog2.SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);

                // c interface
                IGFD_SetFlashingAttenuationInSeconds(cFileDialogPtr, flashingAttenuationInSeconds);
            }
            ImGui::PopItemWidth();
#endif
            ImGui::Separator();
            ImGui::Checkbox("Use file dialog constraint", &UseWindowContraints);
            ImGui::Text("Constraints is used here for define min/max file dialog size");
            ImGui::Separator();

            ImGui::Text("ImGuiFileDialog Flags : ");
            ImGui::Indent();
            {
                ImGui::Text("Commons :");
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Modal", "Open dialog in modal mode", &flags, ImGuiFileDialogFlags_Modal);
                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Overwrite", "Overwrite verification before dialog closing", &flags, ImGuiFileDialogFlags_ConfirmOverwrite);
                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Hide Hidden Files", "Hide Hidden Files", &flags, ImGuiFileDialogFlags_DontShowHiddenFiles);

                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Disable Directory Creation", "Disable Directory Creation button in dialog", &flags, ImGuiFileDialogFlags_DisableCreateDirectoryButton);
#ifdef USE_THUMBNAILS
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Disable thumbnails mode", "Disable thumbnails display in dialo", &flags, ImGuiFileDialogFlags_DisableThumbnailMode);
#endif  // USE_THUMBNAILS
#ifdef USE_PLACES_FEATURE
                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Disable place mode", "Disable place display in dialo", &flags, ImGuiFileDialogFlags_DisablePlaceMode);
#endif  // USE_PLACES_FEATURE

                ImGui::Text("Hide Column by default : (saved in imgui.ini, \n\tso defined when the imgui.ini is not existing)");
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Hide Column Type", "Hide Column file type by default", &flags, ImGuiFileDialogFlags_HideColumnType);
                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Hide Column Size", "Hide Column file Size by default", &flags, ImGuiFileDialogFlags_HideColumnSize);
                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Hide Column Date", "Hide Column file Date by default", &flags, ImGuiFileDialogFlags_HideColumnDate);

                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Case Insensitive Extentions", "will not take into account the case of file extentions", &flags, ImGuiFileDialogFlags_CaseInsensitiveExtention);

                ImGui::SameLine();
                ImGui::RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>(0.0f, "Disable quick path selection", "Disable the quick path selection", &flags, ImGuiFileDialogFlags_DisableQuickPathSelection);

                ImGui::Separator();
                ImGui::Text("Result Modes : for GetFilePathName and GetSelection");

                if (ImGui::RadioButtonLabeled(0.0f, "Add If No File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt, false)) {
                    resultMode = IGFD_ResultMode_::IGFD_ResultMode_AddIfNoFileExt;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, "Overwrite File Ext", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt, false)) {
                    resultMode = IGFD_ResultMode_::IGFD_ResultMode_OverwriteFileExt;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, "Keep Input File", nullptr, resultMode == IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile, false)) {
                    resultMode = IGFD_ResultMode_::IGFD_ResultMode_KeepInputFile;
                }
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Singleton acces :")) {
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog")) {
                const char* filters = ".*,.cpp,.h,.hpp";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters")) {
                const char* filters =
                    "All files {.*},Frames Format 1 (.001,.NNN){(([.][0-9]{3}))},Frames Format 2 (nnn.png){(([0-9]*.png))},Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with filter of type regex ((Custom.+[.]h))")) {
                // the regex for being recognized at regex need to be between ( and )
                const char* filters = "Regex Custom*.h{((Custom.+[.]h))}";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with filter of type regex (([a-zA-Z0-9]+)) for extention less files")) {
                // the regex for being recognized at regex need to be between ( and )
                const char* filters = "Regex ext less {(([a-zA-Z0-9]+))}";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items")) {
                const char* filters = ".*,.cpp,.h,.hpp";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 5;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection")) {
                const char* filters = ".*,.cpp,.h,.hpp";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 0;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with last file path name")) {
                const char* filters = ".*,.cpp,.h,.hpp";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.filePathName      = filePathName;
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open All file types with filter .*")) {
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer filter")) {
                const char* filters = ".a.b";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 0;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer .*.*")) {
                const char* filters = ".*.*";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 0;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with multilayer .vcxproj.*")) {
                const char* filters = ".vcxproj.*";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 0;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with regex (([.]vcx))")) {
                const char* filters = "(([.]vcx))";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 0;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open All file types with a multilayer collectionfilter")) {
                const char* filters = "multi layers{.filters, .a.b }";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_SAVE " Save File Dialog with a custom pane")) {
                const char* filters = "C++ File (*.cpp){.cpp}";
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.sidePane          = std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
                config.sidePaneWidth     = 350.0f;
                config.userDatas         = IGFDUserDatas("SaveFile");
                config.flags             = flags;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open File Dialog with custom size according to gltf files")) {
                const char* filters = ".gltf";
                IGFD::FileDialogConfig config;
                config.path               = ".";
                config.countSelectionMax  = 0;
                config.flags              = flags;
                config.userFileAttributes = [](IGFD::FileInfos* vFileInfosPtr, IGFD::UserDatas /*vUserDatas*/) -> bool {
                    if (vFileInfosPtr != nullptr) {
                        // this demo not take into account .gltf who have data insise. besauce keepd easy just for demo
                        if (vFileInfosPtr->SearchForExt(".gltf", true)) {
                            auto bin_file_path_name = vFileInfosPtr->filePath + IGFD::Utils::GetPathSeparator() + vFileInfosPtr->fileNameLevels[0] + ".bin";
                            struct stat statInfos   = {};
                            int result              = stat(bin_file_path_name.c_str(), &statInfos);
                            if (!result) {
                                vFileInfosPtr->tooltipMessage = toStr("%s : %s\n%s : %s",                                               //
                                                                      (vFileInfosPtr->fileNameLevels[0] + ".gltf").c_str(),             //
                                                                      IGFD::Utils::FormatFileSize(vFileInfosPtr->fileSize).c_str(),     //
                                                                      (vFileInfosPtr->fileNameLevels[0] + ".bin").c_str(),              //
                                                                      IGFD::Utils::FormatFileSize((size_t)statInfos.st_size).c_str());  //
                                vFileInfosPtr->tooltipColumn  = 1;
                                vFileInfosPtr->fileSize += (size_t)statInfos.st_size;
                            } else {
                                // no bin, so escaped.
                                // normally we must parse the file and check the uri for get the buffer file
                                // but here we keep the example as easy for demo.
                                return false;
                            }
                        }
                    }
                    return true;
                };
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
        }

        if (ImGui::CollapsingHeader("Open Directories :")) {
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog")) {
                // let filters be null for open directory chooser
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 1;
                config.flags             = flags;
                fileDialog2.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, config);
            }
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items")) {
                // set filters be null for open directory chooser
                IGFD::FileDialogConfig config;
                config.path              = ".";
                config.countSelectionMax = 5;
                config.flags             = flags;
                fileDialog2.OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, config);
            }
        }

        if (ImGui::CollapsingHeader("Draw Override of the FileDialog for have a read only checkbox")) {
            if (ImGui::ContrastedButton(ICON_IGFD_FOLDER_OPEN " Open A Draww Override FileDialog with a read only btn")) {
                const char* filters = "All files{.*},Frames Format 1(.001,.NNN){(([.][0-9]{3}))},Frames Format 2(nnn.png){(([0-9]*.png))},Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
                IGFD::FileDialogConfig config;
                config.path = ".";
                CustomDrawReadOnlyCheckBoxFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, config);
            }
            ImGui::Text("Is File Read only ?? : %s", IsFileReadOnly ? "yes" : "false");

            /////////////////////////////////////////////////////////////////
            // C Interface
            /////////////////////////////////////////////////////////////////
        }

        if (ImGui::CollapsingHeader("C API instance demo")) {
            if (ImGui::ContrastedButton("C " ICON_IGFD_SAVE " Save File Dialog with a custom pane")) {
                const char* filters           = "C++ File (*.cpp){.cpp}";
                IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();
                config.path                   = ".";
                config.countSelectionMax      = 1;
                config.sidePane               = &InfosPane;
                config.sidePaneWidth          = 350.0f;
                config.userDatas              = (void*)("SaveFile");
                config.flags                  = flags;
                IGFD_OpenDialog(cFileDialogPtr, "ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, config);
            }
        }
        /////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////

        if (ImGui::CollapsingHeader("Embedded Dialog demo :")) {
            IGFD::FileDialogConfig config;
            config.countSelectionMax = -1;
            config.flags             = ImGuiFileDialogFlags_NoDialog |  // permit the embedded, because no frame is used
#ifdef USE_PLACES_FEATURE
                           ImGuiFileDialogFlags_DisablePlaceMode |              // place mode
#endif                                                                          // USE_PLACES_FEATURE
                           ImGuiFileDialogFlags_DisableCreateDirectoryButton |  // no directory creation button
                           ImGuiFileDialogFlags_ReadOnlyFileNameField;          // file name filed is read only
            fileDialogEmbedded3.OpenDialog("embedded", "Select File", ".*", config);
            // to note, when embedded only, the vMinSize do nothing, only the vMaxSize can size the dialog frame
            if (fileDialogEmbedded3.Display("embedded", ImGuiWindowFlags_NoCollapse, ImVec2(0, 0), ImVec2(0, 350))) {
                if (fileDialogEmbedded3.IsOk()) {
                    filePathName = fileDialogEmbedded3.GetFilePathName(resultMode);
                    filePath     = fileDialogEmbedded3.GetCurrentPath();
                    filter       = fileDialogEmbedded3.GetCurrentFilter();
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    if (fileDialogEmbedded3.GetUserDatas()) userDatas = std::string((const char*)fileDialogEmbedded3.GetUserDatas());
                    auto sel = fileDialogEmbedded3.GetSelection(resultMode);  // multiselection
                    selection.clear();
                    for (const auto& s : sel) {
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
                                selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
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

        if (UseWindowContraints) {
            maxSize = ImVec2((float)vDisplayWidth, (float)vDisplayHeight) * 0.7f;
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
                for (const auto& s : sel) {
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
                for (const auto& s : sel) {
                    selection.emplace_back(s.first, s.second);
                }
                // action
            }
            fileDialog2.Close();
        }

        if (CustomDrawReadOnlyCheckBoxFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
            if (CustomDrawReadOnlyCheckBoxFileDialog::Instance()->IsOk()) {
                IsFileReadOnly = CustomDrawReadOnlyCheckBoxFileDialog::Instance()->isReadOnly();
            }
            CustomDrawReadOnlyCheckBoxFileDialog::Instance()->Close();
        }

        /////////////////////////////////////////////////////////////////
        // C Interface
        /////////////////////////////////////////////////////////////////

        if (IGFD_DisplayDialog(cFileDialogPtr, "ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
            if (IGFD_IsOk(cFileDialogPtr)) {
                char* cfilePathName = IGFD_GetFilePathName(cFileDialogPtr, IGFD_ResultMode_AddIfNoFileExt);
                if (cfilePathName) filePathName = cfilePathName;
                char* cfilePath = IGFD_GetCurrentPath(cFileDialogPtr);
                if (cfilePath) filePath = cfilePath;
                char* cfilter = IGFD_GetCurrentFilter(cFileDialogPtr);
                if (cfilter) filter = cfilter;
                // here convert from string because a string was passed as a userDatas, but it can be what you want
                void* cdatas = IGFD_GetUserDatas(cFileDialogPtr);
                if (cdatas) userDatas = (const char*)cdatas;
                IGFD_Selection csel = IGFD_GetSelection(cFileDialogPtr, IGFD_ResultMode_KeepInputFile);  // multiselection

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
            IGFD_CloseDialog(cFileDialogPtr);
        }
    }
    ImGui::End();
}

void DemoDialog::manageGPU() {
#ifdef USE_THUMBNAILS
    ImGuiFileDialog::Instance()->ManageGPUThumbnails();
    fileDialogEmbedded3.ManageGPUThumbnails();
#endif
}

void DemoDialog::unit() {
#ifdef USE_PLACES_FEATURE
    // remove place
    // todo : do the code
    // ImGuiFileDialog::Instance()->RemovePlace("Current Dir");

    // save place dialog 1
    std::ofstream configFileWriter_1("places_1.conf", std::ios::out);
    if (!configFileWriter_1.bad()) {
        configFileWriter_1 << ImGuiFileDialog::Instance()->SerializePlaces();
        configFileWriter_1.close();
    }
    // save place dialog 2
    std::ofstream configFileWriter_2("places_2.conf", std::ios::out);
    if (!configFileWriter_2.bad()) {
        configFileWriter_2 << fileDialog2.SerializePlaces();
        configFileWriter_2.close();
    }
    // save place dialog c interface
    std::ofstream configFileWriter_c("places_c.conf", std::ios::out);
    if (!configFileWriter_c.bad()) {
        char* s = IGFD_SerializePlaces(cFileDialogPtr, true);
        if (s) {
            configFileWriter_c << std::string(s);
            configFileWriter_c.close();
        }
    }
#endif

    // c interface
    IGFD_Destroy(cFileDialogPtr);
}
