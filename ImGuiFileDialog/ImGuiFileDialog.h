/*
MIT License

Copyright (c) 2019-2020 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

github repo : https://github.com/aiekick/ImGuiFileDialog

-----------------------------------------------------------------------------------------------------------------
## Description :
-----------------------------------------------------------------------------------------------------------------

this File Dialog is build on top of DearImGui
(On windows, need te lib Dirent : https://github.com/tronkko/dirent, use the branch 1.23 for avoid any issues)
Complete readme here : https://github.com/aiekick/ImGuiFileDialog/blob/master/README.md)

this filedialog was created principally for have custom pane with widgets accrdoing to file extention.
it was not possible with native filedialog

An example of the File Dialog integrated within the ImGui Demo App

-----------------------------------------------------------------------------------------------------------------
## Features :
-----------------------------------------------------------------------------------------------------------------

- Separate system for call and display
  - can be many func calls with different params for one display func by ex
- Can use custom pane via function binding
  - this pane can block the validation of the dialog
  - can also display different things according to current filter and User Datas
- Support of Filter Custom Coloring / Icons / text
- Multi Selection (ctrl/shift + click) :
  - 0 => infinite
  - 1 => one file (default)
  - n => n files
- Compatible with MacOs, Linux, Win
  - On Win version you can list Drives
- Support of Modal/Standard dialog type
- Support both Mode : File Chooser or Directory Chooser
- Support filter collection / Custom filter name
- Support files Exploring with keys : Up / Down / Enter (open dir) / Backspace (come back)
- Support files Exploring by input char (case insensitive)
- Support bookmark creation/edition/call for directory (can have custom name corresponding to a path)
- Support input path edition by right click on a path button
- Support of a 'Confirm to Overwrite" dialog if File Exist


-----------------------------------------------------------------------------------------------------------------
## NameSpace / SingleTon
-----------------------------------------------------------------------------------------------------------------

Use the Namespace igfd (for avoid conflict with variables, struct and class names)

you can display only one dialog at a time, this class is a simgleton and must be called like that :
igfd::ImGuiFileDialog::Instance()->method_of_your_choice()

-----------------------------------------------------------------------------------------------------------------
## Simple Dialog :
-----------------------------------------------------------------------------------------------------------------

Example code :
void drawGui()
{
  // open Dialog Simple
  if (ImGui::Button("Open File Dialog"))
	igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", ".");

  // display
  if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
  {
	// action if OK
	if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
	{
	  std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
	  std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
	  // action
	}
	// close
	igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
  }
}

-----------------------------------------------------------------------------------------------------------------
## Directory Chooser :
-----------------------------------------------------------------------------------------------------------------

For have only a directory chooser, you just need to specify a filter null :

Example code :
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", 0, ".");

In this mode you can select any directory with one click, and open directory with double click

-----------------------------------------------------------------------------------------------------------------
## Dialog with Custom Pane :
-----------------------------------------------------------------------------------------------------------------

Example code :
static bool canValidateDialog = false;
inline void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
	ImGui::Text("Selected Filter : %s", vFilter.c_str());
	if (vUserDatas)
		ImGui::Text("UserDatas : %s", vUserDatas);
	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
	if (vCantContinue)
		*vCantContinue = canValidateDialog;
}

void drawGui()
{
  // open Dialog with Pane
  if (ImGui::Button("Open File Dialog with a custom pane"))
	igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp",
			".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 350, 1, igfd::UserDatas("InfosPane"));

  // display and action if ok
  if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey"))
  {
	if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
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
	// close
	igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
  }
}

-----------------------------------------------------------------------------------------------------------------
## Filter Infos
-----------------------------------------------------------------------------------------------------------------

You can define color for a filter type
Example code :
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1,1,0, 0.9));
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0,1,0, 0.9));
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0,0,1, 0.9));
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".md", ImVec4(1,0,1, 0.9));


![alt text](doc/color_filter.png)

and also specific icons (with icon font files) or file type names :

Example code :
// add an icon for png files
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".png", ImVec4(0,1,1,0.9), ICON_IMFDLG_FILE_TYPE_PIC);
// add a text for gif files (the default value is [File]
igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".gif", ImVec4(0, 1, 0.5, 0.9), "[GIF]");


![alt text](doc/filter_Icon.png)

-----------------------------------------------------------------------------------------------------------------
## Filter Collections
-----------------------------------------------------------------------------------------------------------------

you can define a custom filter name who correspond to a group of filter

you must use this syntax : custom_name1{filter1,filter2,filter3},custom_name2{filter1,filter2},filter1
when you will select custom_name1, the gorup of filter 1 to 3 will be applied
the reserved char are {}, you cant use them for define filter name.

Example code :
const char *filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IMFDLG_FOLDER_OPEN " Choose a File", filters, ".");

## Multi Selection

You can define in OpenDialog/OpenModal call the count file you wan to select :
- 0 => infinite
- 1 => one file only (default)
- n => n files only

See the define at the end of these funcs after path.

Example code :
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*,.cpp,.h,.hpp", ".");
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose 1 File", ".*,.cpp,.h,.hpp", ".", 1);
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose 5 File", ".*,.cpp,.h,.hpp", ".", 5);
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose many File", ".*,.cpp,.h,.hpp", ".", 0);
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg",
   ".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 350, 1, "SaveFile"); // 1 file

-----------------------------------------------------------------------------------------------------------------
## File Dialog Constraints
-----------------------------------------------------------------------------------------------------------------

you can define min/max size of the dialog when you display It

by ex :

* MaxSize is the full display size
* MinSize in the half display size.

Example code :
ImVec2 maxSize = ImVec2((float)display_w, (float)display_h);
ImVec2 minSize = maxSize * 0.5f;
igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize);

-----------------------------------------------------------------------------------------------------------------
## Detail View Mode
-----------------------------------------------------------------------------------------------------------------

You can have tables display like that.

- uncomment "#define USE_IMGUI_TABLES" in you custom config file (CustomImGuiFileDialogConfig.h in this example)

-----------------------------------------------------------------------------------------------------------------
## Exploring by keys
-----------------------------------------------------------------------------------------------------------------

you can activate this feature by uncomment : "#define USE_EXPLORATION_BY_KEYS"
in you custom config file (CustomImGuiFileDialogConfig.h in this example)

you can also uncomment the next lines for define your keys :

* IGFD_KEY_UP => Up key for explore to the top
* IGFD_KEY_DOWN => Down key for explore to the bottom
* IGFD_KEY_ENTER => Enter key for open directory
* IGFD_KEY_BACKSPACE => BackSpace for comming back to the last directory

you can also explore a file list by use the current key char.

as you see the current item is flashed (by default for 1 sec)
you can define the flashing life time by yourself with the function

Example code :
igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(1.0f);

-----------------------------------------------------------------------------------------------------------------
## Bookmarks
-----------------------------------------------------------------------------------------------------------------

you can create/edit/call path bookmarks and load/save them in file

you can activate it by uncomment : "#define USE_BOOKMARK"

in you custom config file (CustomImGuiFileDialogConfig.h in this example)

you can also uncomment the next lines for customize it :
Example code :
#define bookmarkPaneWith 150.0f => width of the bookmark pane
#define IMGUI_TOGGLE_BUTTON ToggleButton => customize the Toggled button (button stamp must be : (const char* label, bool *toggle)
#define bookmarksButtonString "Bookmark" => the text in the toggle button
#define bookmarksButtonHelpString "Bookmark" => the helper text when mouse over the button
#define addBookmarkButtonString "+" => the button for add a bookmark
#define removeBookmarkButtonString "-" => the button for remove the selected bookmark


* you can select each bookmark for edit the displayed name corresponding to a path
* you must double click on the label for apply the bookmark

you can also serialize/deserialize bookmarks by ex for load/save from/to file : (check the app sample by ex)
Example code :
Load => igfd::ImGuiFileDialog::Instance()->DeserializeBookmarks(bookmarString);
Save => std::string bookmarkString = igfd::ImGuiFileDialog::Instance()->SerializeBookmarks();


-----------------------------------------------------------------------------------------------------------------
## Path Edition :
-----------------------------------------------------------------------------------------------------------------

if you click right on one of any path button, you can input or modify the path pointed by this button.
then press the validate key (Enter by default with GLFW) for validate the new path
or press the escape key (Escape by default with GLFW) for quit the input path edition

see in this gif doc/inputPathEdition.gif :
1) button edition with mouse button right and escape key for quit the edition
2) focus the input and press validation for set path

-----------------------------------------------------------------------------------------------------------------
## Confirm to OverWrite Dialog :
-----------------------------------------------------------------------------------------------------------------

If you want avoid OverWrite your files after confirmation,
you can show a Dialog for confirm or cancel the OverWrite operation.

You just need to define the flag ImGuiFileDialogFlags_ConfirmOverwrite
in your call to OpenDialog/OpenModal

By default this flag is not set, since there is no pre-defined way to
define if a dialog will be for Open or Save behavior. (and its wanted :) )

Example code For Standard Dialog :
Example code :
igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
	ICON_IGFD_SAVE " Choose a File", filters,
	".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);

Example code For Modal Dialog :
Example code :
igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
	ICON_IGFD_SAVE " Choose a File", filters,
	".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);

This dialog will only verify the file in the file field.
So Not to be used with GetSelection()

The Confirm dialog will be a forced Modal Dialog, not moveable, displayed
in the center of the current FileDialog.

As usual you can customize the dialog,
in you custom config file (CustomImGuiFileDialogConfig.h in this example)

you can  uncomment the next lines for customize it :

Example code :
#define OverWriteDialogTitleString "The file Already Exist !"
#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#define OverWriteDialogConfirmButtonString "Confirm"
#define OverWriteDialogCancelButtonString "Cancel"

-----------------------------------------------------------------------------------------------------------------
## Open / Save dialog Behavior :
-----------------------------------------------------------------------------------------------------------------

There is no way to distinguish the "open dialog" behavior than "save dialog" behavior.
So you msut adapt the return according to your need :

if you want open file(s) or directory(s), you must use : GetSelection() method. you will obtain a std::map<FileName, FilePathName> of the selection
if you want create a file, you must use : GetFilePathName()/GetCurrentFileName()

the return method's and comments :

Example code :
std::map<std::string, std::string> GetSelection(); // Open File behavior : will return selection via a map<FileName, FilePathName>
std::string GetFilePathName();                     // Create File behavior : will always return the content of the field with current filter extention and current path
std::string GetCurrentFileName();                  // Create File behavior : will always return the content of the field with current filter extention
std::string GetCurrentPath();                      // will return current path
std::string GetCurrentFilter();                    // get selected filter
UserDatas GetUserDatas();                          // get user datas send with Open Dialog

-----------------------------------------------------------------------------------------------------------------
## How to Integrate ImGuiFileDialog in your project
-----------------------------------------------------------------------------------------------------------------

### ImGuiFileDialog require :

* dirent v1.23 (https://github.com/tronkko/dirent/tree/v1.23) lib, only for windows. Successfully tested with version v1.23 only
* Dear ImGui (https://github.com/ocornut/imgui/tree/master) (with/without tables widgets)

### Customize ImGuiFileDialog :

You just need to write your own config file by override the file : ImGuiFileDialog/ImGuiFileDialogConfig.h
like i do here with CustomImGuiFileDialogConfig.h

After that, for let ImGuiFileDialog your own custom file,
you must define the preprocessor directive CUSTOM_IMGUIFILEDIALOG_CONFIG with the path of you custom config file.
This path must be relative to the directory where you put ImGuiFileDialog module.

-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

Thats all.

You can check by example in this repo with the file CustomImGuiFileDialogConfig.h :
- this trick was used for have custom icon font instead of labels for buttons or messages titles
- you can also use your custom imgui button, the button call stamp must be same by the way :)

The Custom Icon Font (in CustomFont.cpp and CustomFont.h) was made with ImGuiFontStudio (https://github.com/aiekick/ImGuiFontStudio) i wrote for that :)
ImGuiFontStudio is using also ImGuiFileDialog.

-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

My appologies, the v0.5.3 was wrong, not commited the expected good one :(
*/

#pragma once

#define IMGUIFILEDIALOG_VERSION "v0.5.3.1"

#include <imgui.h>

#include <cfloat>
#include <utility>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <list>

#ifndef CUSTOM_IMGUIFILEDIALOG_CONFIG
#include "ImGuiFileDialogConfig.h"
#else
#include CUSTOM_IMGUIFILEDIALOG_CONFIG
#endif

typedef int ImGuiFileDialogFlags; // -> enum ImGuiFileDialogFlags_

enum ImGuiFileDialogFlags_
{
	ImGuiFileDialogFlags_None = 0,
	ImGuiFileDialogFlags_ConfirmOverwrite = 1 << 0,
};

namespace igfd
{
	#ifndef MAX_FILE_DIALOG_NAME_BUFFER 
	#define MAX_FILE_DIALOG_NAME_BUFFER 1024
	#endif

	#ifndef MAX_PATH_BUFFER_SIZE
	#define MAX_PATH_BUFFER_SIZE 1024
	#endif

	typedef void* UserDatas;

	struct FileInfoStruct
	{
		char type = ' ';
		std::string filePath;
		std::string fileName;
		std::string fileName_optimized; // optimized for search => insensitivecase
		std::string ext;
		size_t fileSize = 0; // for sorting operations
		std::string formatedFileSize;
		std::string fileModifDate;
	};

	// old FilterInfosStruct
	struct FileExtentionInfosStruct
	{
		ImVec4 color = ImVec4(0, 0, 0, 0);
		std::string icon;
		FileExtentionInfosStruct() : color(0, 0, 0, 0) { }
		explicit FileExtentionInfosStruct(const ImVec4& vColor, std::string vIcon = std::string()) : color(vColor), icon(std::move(vIcon)){}
	};

	struct FilterInfosStruct
	{
		std::string filter;
		std::set<std::string> collectionfilters;

		void clear()
		{
			filter.clear();
			collectionfilters.clear();
		}

		bool empty() const
		{
			return
				filter.empty() &&
				collectionfilters.empty();
		}

		bool filterExist(const std::string& vFilter)
		{
			return
				filter == vFilter ||
				collectionfilters.find(vFilter) != collectionfilters.end();
		}
	};

	enum class SortingFieldEnum
    {
		FIELD_NONE = 0,
	    FIELD_FILENAME,
	    FIELD_SIZE,
		FIELD_DATE
    };

	struct BookmarkStruct
	{
		std::string name;
		std::string path;
	};

	class ImGuiFileDialog
	{
	private:
		std::vector<FileInfoStruct> m_FileList;
        std::vector<FileInfoStruct> m_FilteredFileList;
        std::unordered_map<std::string, FileExtentionInfosStruct> m_FileExtentionInfos;
		std::string m_CurrentPath;
		std::vector<std::string> m_CurrentPath_Decomposition;
		std::set<std::string> m_SelectedFileNames;
		std::string m_Name;
		bool m_ShowDialog = false;
		bool m_ShowDrives = false;
		std::string m_LastSelectedFileName; // for shift multi selectio
		std::vector<FilterInfosStruct> m_Filters;
		FilterInfosStruct m_SelectedFilter;
		bool m_InputPathActivated = false; // show input for path edition
        ImGuiListClipper m_FileListClipper;
		ImVec2 m_DialogCenterPos = ImVec2(0, 0); // center pos for display the confirm overwrite dialog
		int m_LastImGuiFrameCount = 0; // to be sure than only one dialog displayed per frame
		float m_FooterHeight = 0.0f;

	private: // events
		bool m_DrivesClicked = false;
		bool m_PathClicked = false;
		bool m_CanWeContinue = true;

	private:
#ifdef USE_BOOKMARK
        ImGuiListClipper m_BookmarkClipper;
		std::vector<BookmarkStruct> m_Bookmarks;
		bool m_BookmarkPaneShown = false;
#endif

	private: // flash when select by char
		size_t m_FlashedItem = 0;
		float m_FlashAlpha = 0.0f;
		float m_FlashAlphaAttenInSecs = 1.0f; // fps display dependant
		bool m_OkResultToConfirm = false; // to confim if ok for OverWrite

	public:
		char InputPathBuffer[MAX_PATH_BUFFER_SIZE] = "";
		char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
		char DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
		char SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#ifdef USE_BOOKMARK
		char BookmarkEditBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#endif
		bool IsOk = false;
		bool m_AnyWindowsHovered = false;
		bool m_CreateDirectoryMode = false;

	private:
		std::string dlg_key;
		std::string dlg_name;
		const char *dlg_filters{};
		std::string dlg_path;
		std::string dlg_defaultFileName;
		std::string dlg_defaultExt;
		ImGuiFileDialogFlags dlg_flags = ImGuiFileDialogFlags_None;
		std::function<void(std::string, UserDatas, bool*)> dlg_optionsPane;
		float dlg_optionsPaneWidth = 0.0f;
		std::string searchTag;
		UserDatas dlg_userDatas{};
		size_t dlg_countSelectionMax = 1; // 0 for infinite
		bool dlg_modal = false;
		
	private: // detail view
		std::string m_HeaderFileName;
		std::string m_HeaderFileSize;
		std::string m_HeaderFileDate;
		bool m_SortingDirection[3] = { true,true,true }; // true => Descending, false => Ascending
		SortingFieldEnum m_SortingField = SortingFieldEnum::FIELD_FILENAME;

	public:
		static ImGuiFileDialog* Instance()
		{
			static auto *_instance = new ImGuiFileDialog();
			return _instance;
		}

	protected:
		ImGuiFileDialog(); // Prevent construction
		ImGuiFileDialog(const ImGuiFileDialog&) {}; // Prevent construction by copying
		ImGuiFileDialog& operator =(const ImGuiFileDialog&) { return *this; }; // Prevent assignment
		~ImGuiFileDialog(); // Prevent unwanted destruction

	public: // standard dialog
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);

	public: // modal dialog
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			UserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);

	public: // core
		bool FileDialog(const std::string& vKey, ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse,
			ImVec2 vMinSize = ImVec2(0, 0), ImVec2 vMaxSize = ImVec2(FLT_MAX, FLT_MAX));
		void CloseDialog();

		bool WasOpenedThisFrame(const std::string& vKey);		// say if the dialog key was already opened this frame
		bool IsOpened(const std::string& vKey);					// say if the key is opened
		bool IsOpened();										// say if the dialog is opened somewhere	
		std::string GetOpenedKey();								// return the dialog key who is opened, return nothing if not opened

		std::map<std::string, std::string> GetSelection();		// Open File behavior : will return selection via a map<FileName, FilePathName>
		std::string GetFilePathName();							// Create File behavior : will always return the content of the field with current filter extention and current path
		std::string GetCurrentFileName();						// Create File behavior : will always return the content of the field with current filter extention
		std::string GetCurrentPath();							// will return current path
		std::string GetCurrentFilter();							// get selected filter
		UserDatas GetUserDatas();								// get user datas send with Open Dialog
		
		void SetExtentionInfos(const std::string& vFilter, const FileExtentionInfosStruct& vInfos);
		void SetExtentionInfos(const std::string& vFilter, const ImVec4& vColor, const std::string& vIcon = "");
		bool GetExtentionInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon = nullptr);
		void ClearExtentionInfos();

	private: // dialog parts
		void DrawHeader();
		void DrawContent();
		bool DrawFooter();

	private: // widgets components
#ifdef USE_BOOKMARK
		void DrawBookMark();
#endif
		void DrawDirectoryCreation();
		void DrawPathComposer();
		void DrawSearchBar();
		void DrawFileListView(ImVec2 vSize);
		void DrawSidePane(float vHeight);

	private:
		void ResetEvents();
		void SetDefaultFileName(const std::string& vFileName);
		bool SelectDirectory(const FileInfoStruct& vInfos);
		void SelectFileName(const FileInfoStruct& vInfos);
		void RemoveFileNameInSelection(const std::string& vFileName);
		void AddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName);
		void SetPath(const std::string& vPath);
		void FillInfos(FileInfoStruct *vFileInfoStruct);
		void SortFields(SortingFieldEnum vSortingField = SortingFieldEnum::FIELD_NONE, bool vCanChangeOrder = false);
		void ScanDir(const std::string& vPath);
		void SetCurrentDir(const std::string& vPath);
		bool CreateDir(const std::string& vPath);
		std::string ComposeNewPath(std::vector<std::string>::iterator vIter);
		void GetDrives();
		void ParseFilters(const char *vFilters);
		void SetSelectedFilterWithExt(const std::string& vFilter);
		static std::string OptimizeFilenameForSearchOperations(std::string vFileName);

	private:
	    void ApplyFilteringOnFileList();

#ifdef USE_EXPLORATION_BY_KEYS
	private: // file localization by input chat // widget flashing
		void LocateByInputKey();
		bool LocateItem_Loop(ImWchar vC);
		void ExploreWithkeys();
		static bool FlashableSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
			bool vFlashing = false, const ImVec2& size = ImVec2(0, 0));
		void StartFlashItem(size_t vIdx);
		bool BeginFlashItem(size_t vIdx);
		void EndFlashItem();
	public:
		void SetFlashingAttenuationInSeconds(float vAttenValue);
#endif
#ifdef USE_BOOKMARK
	private:
		void DrawBookmarkPane(ImVec2 vSize);
	public:
		std::string SerializeBookmarks();
		void DeserializeBookmarks(const std::string& vBookmarks);
#endif

	private: // Overwrite Dialog
		bool Confirm_Or_OpenOverWriteFileDialog_IfNeeded(bool vLastAction, ImGuiWindowFlags vFlags);
		bool IsFileExist(const std::string& vFile);
	};
}
