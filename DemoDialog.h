#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>

class CustomDrawReadOnlyCheckBoxFileDialog : public ImGuiFileDialog {
private:
    bool m_ReadOnly = false;

public:
    static CustomDrawReadOnlyCheckBoxFileDialog* Instance(CustomDrawReadOnlyCheckBoxFileDialog* vCopy = nullptr, bool vForce = false);

public:
    void OpenDialog(const std::string& vKey, const std::string& vTitle, const char* vFilters, const IGFD::FileDialogConfig& vConfig) override;
    bool isReadOnly() const;

protected:
    bool m_DrawFooter() override;
};

class DemoDialog {
private:
    bool show_demo_window = true;
    ImGuiFileDialog fileDialog2;
    ImGuiFileDialog fileDialogEmbedded3;
    ImGuiFileDialog* cFileDialogPtr = nullptr;
    CustomDrawReadOnlyCheckBoxFileDialog customFileDialog;

public:
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:
    void init(const float& vFontSize);
    void display(const int32_t& vDisplayWidth, const int32_t& vDisplayHeight);
    void manageGPU();
    void unit();
};