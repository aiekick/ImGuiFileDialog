#include <tests/DialogTests.h>

#include <memory>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <imgui_test_engine/imgui_te_context.h>

class IGFDTester {
public:
    static void init() {
        internal_ptr() = std::make_unique<ImGuiFileDialog>();
    }
    static ImGuiFileDialog& ref() {
        return *internal_ptr().get();
    }
    static void unit() {
        internal_ptr().reset();
    }
    static void reinit() {
        unit();
        init();
    }

private:
    static std::unique_ptr<ImGuiFileDialog>& internal_ptr() {
        static std::unique_ptr<ImGuiFileDialog> mp_instance;
        return mp_instance;
    }
};

void RegisterIGFDDialogTests(ImGuiTestEngine* e) {

    ImGuiTest* t = nullptr;

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog and resize");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        if (ctx->IsFirstGuiFrame()) {
            IGFDTester::reinit();
        }
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFDTester::ref().OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::End();
        if (IGFDTester::ref().Display("test", ImGuiWindowFlags_NoSavedSettings)) {
            IGFDTester::ref().Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->WindowResize("//Choose a File##test", ImVec2(500, 300));
    };

    t          = IM_REGISTER_TEST(e, "dialog_tests", "open dialog, resize, select a path and a file");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        if (ctx->IsFirstGuiFrame()) {
            IGFDTester::reinit();
        }
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY "/unicode";
            IGFDTester::ref().OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::End();
        if (IGFDTester::ref().Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(), ImVec2(500, 300))) {
            IGFDTester::ref().Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->SetRef("//Choose a File##test");
        ctx->ItemClick("**/samples");
        ImGuiWindow* fileTable = ctx->WindowInfo("FileDialog_fileTable").Window;
        IM_CHECK(fileTable != nullptr);
        ctx->WindowFocus(fileTable->ID);
        ImGui::SetScrollX(fileTable, 0);
        ImGui::SetScrollY(fileTable, 0);
        ctx->Yield();
        ctx->MouseSetViewport(fileTable);
        ctx->MouseMoveToPos(fileTable->Rect().GetCenter());
        IM_CHECK_EQ(fileTable->Scroll.x, 0.0f);
        IM_CHECK_EQ(fileTable->Scroll.y, 0.0f);
        ctx->MouseWheelY(5.0f);  // Scroll down
        IM_CHECK_EQ(fileTable->Scroll.x, 0.0f);
        IM_CHECK_GT(fileTable->Scroll.y, 0.0f);


    };
}
