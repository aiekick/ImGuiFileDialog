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

    //-----------------------------------------------------------------
    // ## Demo Test: Hello Automation World
    //-----------------------------------------------------------------

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        if (ctx->IsFirstGuiFrame()) {
            IGFDTester::reinit();
        }
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFDTester::ref().OpenDialog("test", "Choose a File", ".*");
        }
        ImGui::End();

        if (IGFDTester::ref().Display("test", ImGuiWindowFlags_NoSavedSettings)) {
            if (IGFDTester::ref().IsOk()) {
            }
            IGFDTester::ref().Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        // open dialog
        ctx->ItemClick("Open Dialog");  // relative to ref
        // resize
        ctx->WindowResize("//Choose a File##test", ImVec2(300, 300)); // we start with // for be relative to root
        //IM_CHECK(equal(vars.Color1, ImVec4(ImColor(0x11, 0x22, 0x33, 0xFF))));
    };

}
