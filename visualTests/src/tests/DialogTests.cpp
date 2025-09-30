#include <tests/DialogTests.h>

#include <memory>
#include <iostream>

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

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog, resize, select a path and a file");
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
        if (IGFDTester::ref().Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            IGFDTester::ref().Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        // https://github.com/ocornut/imgui_test_engine/wiki/Named-References#using-windowinfo-to-easily-access-child-windows
        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").Window);
        // https://github.com/ocornut/imgui_test_engine/wiki/Named-References#using-123-to-easily-encode-pushid-integers-in-a-string
        ctx->ItemClick("$$6/samples");
        ctx->Yield();
        //std::string path = "//" + std::string(ctx->RefStr)  + "/FileTable";
        //ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent/FileTable").Window);
        ImGuiTestItemList lst;
        ctx->GatherItems(&lst, ctx->GetRef());
        for (const auto it : lst) {
            std::cout << it.DebugLabel << "\n";
        }
        ImGuiTable* fileTable = ImGui::TableFindByID(ctx->GetID("FileTable"));
        IM_CHECK(fileTable != nullptr);


    };
}

/*
t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_boxselect_2");
    struct BoxSelectTestVars { ImGuiTableFlags TableFlags = ImGuiTableFlags_ScrollY; ImGuiSelectionBasicStorage Selection; bool FrozenHeaders = false; };
    t->SetVarsDataType<BoxSelectTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetVars<BoxSelectTestVars>();
        ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::CheckboxFlags("BordersOuter", &vars.TableFlags, ImGuiTableFlags_BordersOuter);
        ImGui::SameLine();
        ImGui::CheckboxFlags("BordersInner", &vars.TableFlags, ImGuiTableFlags_BordersInner);
        if (ImGui::BeginTable("table1", 1, vars.TableFlags))
        {
            auto ms = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_BoxSelect1d, vars.Selection.Size, 1000);
            vars.Selection.ApplyRequests(ms);
            if (vars.FrozenHeaders)
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
            }
            for (unsigned i = 0; i < 1000; i++)
            {
                char buf[32];
                snprintf(buf, sizeof buf, "Item %03d", i);
                ImGui::SetNextItemSelectionUserData(i);
                ImGui::TableNextColumn();
                ImGui::Selectable(buf, vars.Selection.Contains(i));
            }
            ms = ImGui::EndMultiSelect();
            vars.Selection.ApplyRequests(ms);
            ImGui::EndTable();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetVars<BoxSelectTestVars>();
        ImGuiTable* table = ImGui::TableFindByID(ctx->GetID("Test Window/table1"));
        IM_CHECK(table != NULL);

        for (int step = 0; step < 4; step++)
        {
            vars.Selection.Clear();
            vars.TableFlags = (step & 1) ? (vars.TableFlags | ImGuiTableFlags_BordersOuter) : (vars.TableFlags & ~ImGuiTableFlags_BordersOuter);
            vars.FrozenHeaders = (step & 2) != 0;

            ctx->SetRef(table->ID);
            ctx->MouseMove("Item 001");
            ctx->MouseDown(ImGuiMouseButton_Left);
            //ctx->SetRef("//$FOCUSED");
            ctx->MouseMoveToPos(ImGui::GetIO().MousePos + ImVec2(0, ctx->GetWindowByRef("//Test Window")->Size.y));
            ctx->SleepNoSkip(1.0f, 0.05f);
            ctx->MouseUp(ImGuiMouseButton_Left);
            IM_CHECK(vars.Selection.Contains(0) == false);
            int first_selected = 1;
            int last_selected = 1;
            for (int n = 1; n < 1000; n++)
            {
                if (vars.Selection.Contains(n) == false)
                    break;
                last_selected = n;
            }
            IM_CHECK(vars.Selection.Size > 1);
            IM_CHECK_EQ(vars.Selection.Size, last_selected - first_selected + 1); // Check no selection gap (#7970)
        }
    };
*/