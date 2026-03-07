#include <tests/DialogTests.h>

#include <memory>
#include <iostream>
#include <string>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <imgui_test_engine/imgui_te_context.h>

struct IGFDTestVars {
    std::string resultPath;
    std::string resultFileName;
    std::string resultFilter;
    bool wasOk = false;
};

void RegisterIGFDDialogTests(ImGuiTestEngine* e) {

    ImGuiTest* t = nullptr;

    ////////////////////////////////////////////////////////////////////////////
    // Test 1 : open dialog and resize
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog and resize");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings)) {
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->WindowResize("//Choose a File##test", ImVec2(500, 300));
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 2 : open dialog, navigate to a directory, select a file, validate
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog, select a file and validate");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        // show results for debug
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("Path: %s", vars.resultPath.c_str());
        ImGui::Text("File: %s", vars.resultFileName.c_str());
        ImGui::Text("Filter: %s", vars.resultFilter.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultPath = IGFD::GetCurrentPath();
                vars.resultFileName = IGFD::GetFilePathName();
                vars.resultFilter = IGFD::GetCurrentFilter();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();

        // reset results
        vars.wasOk = false;
        vars.resultPath.clear();
        vars.resultFileName.clear();
        vars.resultFilter.clear();

        // open the dialog
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        // set ref to the dialog child content window
        // https://github.com/ocornut/imgui_test_engine/wiki/Named-References#using-windowinfo-to-easily-access-child-windows
        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // type the filename in the input field and press Enter to validate
        // (ItemInputValue uses KeyCharsReplaceEnter which validates the dialog)
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(2);

        // check the results
        IM_CHECK(vars.wasOk == true);
        IM_CHECK(!vars.resultFileName.empty());
        // the result should contain toto.cpp
        IM_CHECK(vars.resultFileName.find("toto.cpp") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 3 : open dialog, navigate into subdirectory, select a file
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog, navigate subdir and select file");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("File: %s", vars.resultFileName.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultPath = IGFD::GetCurrentPath();
                vars.resultFileName = IGFD::GetFilePathName();
                vars.resultFilter = IGFD::GetCurrentFilter();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();

        // reset
        vars.wasOk = false;
        vars.resultFileName.clear();

        // open dialog
        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        // navigate into "insensitive" subdirectory by clicking on it
        // (with NavEnableKeyboard, single click on a dir navigates into it)
        // selectables use the format: "[Dir] dirname" or "[File] filename"
        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        ctx->ItemClick("**/[Dir] insensitive");
        ctx->Yield(3);  // wait for directory listing to refresh

        // type the filename in the input field and press Enter to validate
        ctx->ItemInputValue("**/##FileName", "toto_ext_lower_case.cpp");
        ctx->Yield(2);

        // check results
        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultFileName.find("toto_ext_lower_case.cpp") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 4 : open dialog and cancel
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "open dialog and cancel");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        // click Cancel
        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);
        ctx->ItemClick("**/Cancel##aalidationdialog");
        ctx->Yield(2);

        // check that wasOk is false (dialog was cancelled)
        IM_CHECK(vars.wasOk == false);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 5 : open dialog with .cpp filter, verify filtering
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "filter by cpp extension");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".cpp", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("File: %s", vars.resultFileName.c_str());
        ImGui::Text("Filter: %s", vars.resultFilter.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultFileName = IGFD::GetFilePathName();
                vars.resultFilter = IGFD::GetCurrentFilter();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultFileName.clear();
        vars.resultFilter.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // with .cpp filter, only .cpp files should be shown (+ directories)
        // few items total = no clipping issue for GatherItems
        ImGuiTestItemList items;
        ctx->GatherItems(&items, ctx->GetRef());

        bool foundCpp = false;
        bool foundPng = false;
        for (const auto& it : items) {
            std::string label(it.DebugLabel);
            if (label.find("[File] toto.cpp") != std::string::npos)
                foundCpp = true;
            if (label.find("[File] 0001.png") != std::string::npos)
                foundPng = true;
        }
        IM_CHECK(foundCpp == true);   // .cpp file should be visible
        IM_CHECK(foundPng == false);  // .png file should be filtered out

        // select toto.cpp and validate
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultFilter == ".cpp");
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 6 : navigate into subdirectory and back with ..
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "navigate parent directory with dotdot");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("Path: %s", vars.resultPath.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultPath = IGFD::GetCurrentPath();
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultPath.clear();
        vars.resultFileName.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // navigate into "insensitive" subdirectory
        ctx->ItemClick("**/[Dir] insensitive");
        ctx->Yield(3);

        // navigate back to parent via ".."
        ctx->ItemClick("**/[Dir] ..");
        ctx->Yield(3);

        // verify we're back in the samples directory
        // by checking that "insensitive" dir is visible again
        ImGuiTestItemList items;
        ctx->GatherItems(&items, ctx->GetRef());
        bool foundInsensitive = false;
        for (const auto& it : items) {
            std::string label(it.DebugLabel);
            if (label.find("[Dir] insensitive") != std::string::npos)
                foundInsensitive = true;
        }
        IM_CHECK(foundInsensitive == true);

        // validate with a file to confirm we're in the right directory
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultPath.find("samples") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 7 : directory selection mode (nullptr filter)
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "directory selection mode");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a Dir", nullptr, config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("Path: %s", vars.resultPath.c_str());
        ImGui::Text("File: %s", vars.resultFileName.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultPath = IGFD::GetCurrentPath();
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultPath.clear();
        vars.resultFileName.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a Dir##test/childContent").ID);

        // in directory mode, only directories should be visible (no files)
        ImGuiTestItemList items;
        ctx->GatherItems(&items, ctx->GetRef());

        bool foundFile = false;
        bool foundDir = false;
        for (const auto& it : items) {
            std::string label(it.DebugLabel);
            if (label.find("[File]") != std::string::npos)
                foundFile = true;
            if (label.find("[Dir] insensitive") != std::string::npos)
                foundDir = true;
        }
        IM_CHECK(foundFile == false);  // no files in directory mode
        IM_CHECK(foundDir == true);    // directories should be visible

        // type the directory name in the input field and press Enter to validate
        // (##FileName InputText exists in dir mode, Enter triggers isOk = true)
        ctx->ItemInputValue("**/##FileName", "insensitive");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultPath.find("samples") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 8 : search bar filtering
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "search bar filtering");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("File: %s", vars.resultFileName.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultFileName.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // type "toto" in the search field to filter files
        ctx->ItemClick("**/##InputImGuiFileDialogSearchField");
        ctx->KeyCharsReplace("toto");
        ctx->Yield(2);

        // gather items and verify filtering
        ImGuiTestItemList items;
        ctx->GatherItems(&items, ctx->GetRef());

        bool foundToto = false;
        bool foundPng = false;
        for (const auto& it : items) {
            std::string label(it.DebugLabel);
            if (label.find("[File] toto.cpp") != std::string::npos)
                foundToto = true;
            if (label.find("[File] 0001.png") != std::string::npos)
                foundPng = true;
        }
        IM_CHECK(foundToto == true);   // toto.cpp should match search "toto"
        IM_CHECK(foundPng == false);   // 0001.png should be filtered out

        // validate with toto.cpp
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultFileName.find("toto.cpp") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 9 : breadcrumb path navigation
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "breadcrumb path navigation");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".*", config);
        }
        ImGui::Text("Was Ok: %s", vars.wasOk ? "true" : "false");
        ImGui::Text("Path: %s", vars.resultPath.c_str());
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultPath = IGFD::GetCurrentPath();
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultPath.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // navigate into "insensitive" subdirectory first
        ctx->ItemClick("**/[Dir] insensitive");
        ctx->Yield(3);

        // now click on "samples" in the breadcrumb to navigate back
        // (breadcrumb shows: ...\ samples \ insensitive)
        ctx->ItemClick("**/samples");
        ctx->Yield(3);

        // verify we navigated back: "insensitive" should be a dir in the list
        ImGuiTestItemList items;
        ctx->GatherItems(&items, ctx->GetRef());
        bool foundInsensitive = false;
        for (const auto& it : items) {
            std::string label(it.DebugLabel);
            if (label.find("[Dir] insensitive") != std::string::npos)
                foundInsensitive = true;
        }
        IM_CHECK(foundInsensitive == true);

        // validate
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultPath.find("samples") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 10 : double-click on a file should validate the dialog
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "double click file validates");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            IGFD::OpenDialog("test", "Choose a File", ".cpp", config);
        }
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultFileName.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // double-click on a file should validate the dialog directly
        ctx->ItemDoubleClick("**/[File] toto.cpp");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultFileName.find("toto.cpp") != std::string::npos);
    };

    ////////////////////////////////////////////////////////////////////////////
    // Test 11 : confirm overwrite dialog
    ////////////////////////////////////////////////////////////////////////////

    t = IM_REGISTER_TEST(e, "dialog_tests", "confirm overwrite dialog");
    t->SetVarsDataType<IGFDTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        ImGui::Begin("Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Open Dialog")) {
            IGFD::FileDialogConfig config;
            config.path = SAMPLES_DIRECTORY;
            config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
            IGFD::OpenDialog("test", "Choose a File", ".cpp", config);
        }
        ImGui::End();
        if (IGFD::Display("test", ImGuiWindowFlags_NoSavedSettings, ImVec2(500, 300), ImVec2(1000, 600))) {
            if (IGFD::IsOk()) {
                vars.wasOk = true;
                vars.resultFileName = IGFD::GetFilePathName();
            }
            IGFD::Close();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<IGFDTestVars>();
        vars.wasOk = false;
        vars.resultFileName.clear();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Open Dialog");
        ctx->Yield(2);

        ctx->SetRef(ctx->WindowInfo("//Choose a File##test/childContent").ID);

        // type an existing filename => ItemInputValue presses Enter => triggers Ok
        // but ConfirmOverwrite flag will intercept and show the confirm dialog
        ctx->ItemInputValue("**/##FileName", "toto.cpp");
        ctx->Yield(3);

        // the overwrite confirm dialog should be open
        // popup name: "The selected file already exists!##Choose a FiletestOverWriteDialog"
        ctx->SetRef("//The selected file already exists!##Choose a FiletestOverWriteDialog");
        ctx->ItemClick("Confirm");
        ctx->Yield(2);

        IM_CHECK(vars.wasOk == true);
        IM_CHECK(vars.resultFileName.find("toto.cpp") != std::string::npos);
    };
}
