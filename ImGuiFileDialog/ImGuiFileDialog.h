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
github repo : https://github.com/aiekick/ImGuiFileDialog

Description :
this File Dialog is build on top of DearImGui
(On windows, need te lib Dirent : https://github.com/tronkko/dirent, use the branch 1.23 for avoid any issues)
Complete readme here : https://github.com/aiekick/ImGuiFileDialog/blob/master/README.md)

this filedialog was created principally for have custom pane with widgets.
it was not possible with native filedialog

The possibilities are :
- Separate system for call and display
- Can use custom pane via function binding
    this pane can block the validation of the dialog
    can also display different things according to current filter and User Datas
- Support of Filter Custom Coloring / Icons / text
- Multi Selection (ctrl/shift + click) :
    0 => infinite
    1 => one file (default)
    n => n files
- Compatible with MacOs, Linux, Win
    On Win version you can list Drives
- Support of Modal/Standard dialog type
- Support both Mode : File Chooser or Directory Chooser
- Support filter collection / Custom filter name
- Support files Exploring with keys : Up / Down / Enter (open dir) / Backspace (come back)
- Support files Exploring by input char (case insensitive)
- Support bookmark creation/edition/call for directory (can have custom name corresponding to a path)
- Support input path edition by right click on a path button

Use the Namespace igfd (for avoid conflict with variables, struct and class names)

you can display only one dialog at a time, this class is a simgleton and called like that :
igfd::ImGuiFileDialog::Instance()->method_of_your_choice()

its a bit long but you can use a defime if you want like :
#define fdi igfd::ImGuiFileDialog::Instance()
for call any func like fdi->method_of_your_choice()

the filter syntax is a list of filter ext with the '.' :
syntax : ".filter1, .filter2, .filter3"
ex : ".cpp,.h,.hpp"

you can define collections of filters by put filters in {} :
syntax : "collection1_name{.filter1, .filter2, .filter3}, collection2_name{.filter4, .filter5, .filter6}
ex : "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";

you can customize ImGuiFileDialog with a config file here : ImGuiFileDialog/ImGuiFileDialogConfig.h
you can use your own and define the path of your custom config file realtive to the ImGuiFileDialog directory with this define :
#define CUSTOM_IMGUIFILEDIALOG_CONFIG relative_path_to_IGFD_dir\my_config_file.h

*/
#pragma once

#define IMGUIFILEDIALOG_VERSION "v0.5.1"

#include <imgui.h>

#include <float.h>

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
		FileExtentionInfosStruct(const ImVec4& vColor, const std::string& vIcon = std::string()) : color(vColor), icon(vIcon){}
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

		bool empty()
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

	enum SortingFieldEnum
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
		std::set<std::string> m_SelectedFileNames;
		std::string m_CurrentPath;
		std::vector<std::string> m_CurrentPath_Decomposition;
		std::string m_Name;
		bool m_ShowDialog = false;
		bool m_ShowDrives = false;
		std::string m_LastSelectedFileName; // for shift multi selectio
		std::vector<FilterInfosStruct> m_Filters;
		FilterInfosStruct m_SelectedFilter;
		bool m_InputPathActivated = false; // show input for path edition
		
#ifdef USE_BOOKMARK
		std::vector<BookmarkStruct> m_Bookmarks;
		bool m_BookmarkPaneShown = false;
#endif

	private: // flash when select by char
		size_t m_FlashedItem = 0;
		float m_FlashAlpha = 0.0f;
		float m_FlashAlphaAttenInSecs = 1.0f; // fps display dependant

	public:
		static char InputPathBuffer[MAX_PATH_BUFFER_SIZE];
		static char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
		static char DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
		static char SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
#ifdef USE_BOOKMARK
		static char BookmarkEditBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
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
		std::function<void(std::string, UserDatas, bool*)> dlg_optionsPane;
		size_t dlg_optionsPaneWidth = 0;
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
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t&  vOptionsPaneWidth = 250,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t&  vOptionsPaneWidth = 250,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			UserDatas vUserDatas = 0);

	public: // modal dialog
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t&  vOptionsPaneWidth = 250,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t&  vOptionsPaneWidth = 250,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, UserDatas vUserDatas = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			UserDatas vUserDatas = 0);

	public: // core
		bool FileDialog(const std::string& vKey, ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse,
			ImVec2 vMinSize = ImVec2(0, 0), ImVec2 vMaxSize = ImVec2(FLT_MAX, FLT_MAX));
		void CloseDialog(const std::string& vKey);

		std::string GetFilePathName();
		std::string GetCurrentPath();
		std::string GetCurrentFileName();
		std::string GetCurrentFilter();
		UserDatas GetUserDatas();
		std::map<std::string, std::string> GetSelection(); // return map<FileName, FilePathName>

		void SetExtentionInfos(const std::string& vFilter, const FileExtentionInfosStruct& vInfos);
		void SetExtentionInfos(const std::string& vFilter, const ImVec4& vColor, const std::string& vIcon = "");
		bool GetExtentionInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon = 0);
		void ClearExtentionInfos();

	private:
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
		std::string OptimizeFilenameForSearchOperations(std::string vFileName);

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
	public:
		void DrawBookmarkPane(ImVec2 vSize);
		std::string SerializeBookmarks();
		void DeserializeBookmarks(const std::string& vBookmarks);
#endif
	};
}
