// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include "3rdparty/imgui/examples/imgui_impl_opengl3.h"
#include "3rdparty/imgui/examples/imgui_impl_glfw.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include "ImGuiFileDialog.h"

#include "CustomFont.cpp"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
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

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static bool canValidateDialog = false;

inline void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
	
	ImGui::Text("Selected Filter : %s", vFilter.c_str());

	const char* userDatas = (const char*)vUserDatas;
	if (userDatas)
        ImGui::Text("User Datas : %s", userDatas);

	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

	if (vCantContinue)
	    *vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, bool active, bool disabled)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	float w = CalcItemWidth();
	if (w == window->ItemWidthDefault)	w = 0.0f; // no push item width
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	ImVec2 bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
	bb_size.x = ImMax(w, bb_size.x);

	const ImRect check_bb(
		window->DC.CursorPos,
		window->DC.CursorPos + bb_size);
	ItemSize(check_bb, style.FramePadding.y);

	if (!ItemAdd(check_bb, id))
		return false;

	// check
	bool pressed = false;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(check_bb, id, &hovered, &held);

		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
		if (active)
		{
			const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
		}
	}

	// circle shadow + bg
	if (style.FrameBorderSize > 0.0f)
	{
		window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
		window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
	}

	if (label_size.x > 0.0f)
	{
		RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
	}

	return pressed;
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

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
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontAllowUserScaling = true; // zoom wiht ctrl + mouse wheel 

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

	// load icon font file (CustomFont.cpp)
	ImGui::GetIO().Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]"); // add an text for a filter type

#ifdef USE_BOOKMARK
	// load bookmarks
	std::ifstream docFile("bookmarks.conf", std::ios::in);
	if (docFile.is_open())
	{
		std::stringstream strStream;
		strStream << docFile.rdbuf();//read the file
		igfd::ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
		docFile.close();
	}
#endif

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
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
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
			ImGui::Separator();

			ImGui::Text("imGuiFileDialog Demo %s : ", IMGUIFILEDIALOG_VERSION);
			ImGui::Indent();
			{
#ifdef USE_EXPLORATION_BY_KEYS
				static float flashingAttenuationInSeconds = 1.0f;
				if (ImGui::Button("R##resetflashlifetime"))
				{
					flashingAttenuationInSeconds = 1.0f;
					igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(200);
				if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f))
					igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
				ImGui::PopItemWidth();
#endif
				static bool _UseWindowContraints = true;
				ImGui::Separator();
				ImGui::Checkbox("Use file dialog constraint", &_UseWindowContraints);
				ImGui::Text("Constraints is used here for define min/ax fiel dialog size");
				ImGui::Separator();
				static bool standardDialogMode = true;
				ImGui::Text("Open Mode : ");
				ImGui::SameLine();
				if (RadioButtonLabeled("Standard", standardDialogMode, false)) standardDialogMode = true;
				ImGui::SameLine();
				if (RadioButtonLabeled("Modal", !standardDialogMode, false)) standardDialogMode = false;

				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", 
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", 
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters"))
				{
					const char *filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
				}
				if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection"))
				{
					const char *filters = ".*,.cpp,.h,.hpp";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
				}
				if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane"))
				{
					const char *filters = "C++ File (*.cpp){.cpp}";
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
							ICON_IGFD_SAVE " Choose a File", filters,
							".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, 
							std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
							ICON_IGFD_SAVE " Choose a File", filters,
							".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
								std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
				}
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog"))
                {
					// set filters to 0 for open directory chooser
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
                }
                if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items"))
                {
					// set filters to 0 for open directory chooser
					if (standardDialogMode)
						igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
					else
						igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
							ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
                }

                ImVec2 minSize = ImVec2(0, 0);
				ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

				if (_UseWindowContraints)
				{
					maxSize = ImVec2((float)display_w, (float)display_h);
					minSize = maxSize * 0.5f;
				}

				// you can define your flags and min/max window size (theses three settings ae defined by default :
				// flags => ImGuiWindowFlags_NoCollapse
				// minSize => 0,0
				// maxSize => FLT_MAX, FLT_MAX (defined is float.h)

				if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey",
				        ImGuiWindowFlags_NoCollapse, minSize, maxSize))
				{
					if (igfd::ImGuiFileDialog::Instance()->IsOk)
					{
						std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
						std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
						std::string filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
						// here convert from string because a string was passed as a userDatas, but it can be what you want
                        std::string userDatas;
						if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
                            userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
						auto selection = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection

						// action
					}
					igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
				}

				if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseDirDlgKey",
				        ImGuiWindowFlags_NoCollapse, minSize, maxSize))
                {
                    if (igfd::ImGuiFileDialog::Instance()->IsOk)
                    {
                        std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                        std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
                        std::string filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
                        // here convert from string because a string was passed as a userDatas, but it can be what you want
                        std::string userDatas;
                        if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
                            userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
                        auto selection = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection

                        // action
                    }
                    igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseDirDlgKey");
                }
			}
			ImGui::Unindent();

			ImGui::Separator();
			ImGui::Text("Window mode :");
			ImGui::Separator();

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

		// Rendering
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

#ifdef USE_BOOKMARK
	// save bookmarks
	std::ofstream configFileWriter("bookmarks.conf", std::ios::out);
	if (!configFileWriter.bad())
	{
		configFileWriter << igfd::ImGuiFileDialog::Instance()->SerializeBookmarks();
		configFileWriter.close();
	}
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
