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

#pragma once

#define IMGUIFILEDIALOG_VERSION "v0.2"

#include <imgui.h>

#include <float.h>

#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>

#include <future>
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
	#define MAX_FILE_DIALOG_NAME_BUFFER 1024

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
		std::string icon;
		ImVec4 color = ImVec4(0, 0, 0, 0);

		FileExtentionInfosStruct() { color = ImVec4(0, 0, 0, 0); }
		FileExtentionInfosStruct(const ImVec4& vColor, const std::string& vIcon = std::string())
		{
			color = vColor;
			icon = vIcon;
		}
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

		bool filterExist(std::string vFilter)
		{
			return
				filter == vFilter ||
				collectionfilters.find(vFilter) != collectionfilters.end();
		}
	};

	enum SortingFieldEnum
    {
        FIELD_NONE=0,
	    FIELD_FILENAME,
	    FIELD_SIZE,
		FIELD_DATE
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

	private: // flash when select by char
		size_t m_FlashedItem = 0;
		float m_FlashAlpha = 0.0f;
		float m_FlashAlphaStep = 0.01f; // fps display dependant

	public:
		static char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
		static char DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
		static char SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
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
		
	private:
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

		bool FileDialog(const std::string& vKey, ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse,
			ImVec2 vMinSize = ImVec2(0, 0), ImVec2 vMaxSize = ImVec2(FLT_MAX, FLT_MAX));
		void CloseDialog(const std::string& vKey);

		std::string GetFilepathName();
		std::string GetCurrentPath();
		std::string GetCurrentFileName();
		std::string GetCurrentFilter();
		UserDatas GetUserDatas();
		std::map<std::string, std::string> GetSelection(); // return map<FileName, FilePathName>

		void SetExtentionInfos(const std::string& vFilter, FileExtentionInfosStruct vInfos);
		void SetExtentionInfos(const std::string& vFilter, ImVec4 vColor, std::string vIcon = "");
		bool GetExtentionInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon = 0);
		void ClearExtentionInfos();

	private:
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
		void ComposeNewPath(std::vector<std::string>::iterator vIter);
		void GetDrives();
		void ParseFilters(const char *vFilters);
		void SetSelectedFilterWithExt(const std::string& vFilter);
		std::string OptimizeFilenameForSearchOperations(std::string vFileName);

	private:
	    void ApplyFilteringOnFileList();

	private: // file localization by input chat // widget flashing
		void LocateByInputKey();
		void ExploreWithkeys();
		static bool FlashableSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
			bool vFlashing = false, const ImVec2& size = ImVec2(0, 0));
		void StartFlashItem(size_t vIdx);
		bool BeginFlashItem(size_t vIdx);
		void EndFlashItem();
	};
}