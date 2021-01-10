#pragma once

#define IMGUIFILEDIALOG_VERSION "v0.5.4"

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
typedef void* IGFDUserDatas;

namespace ImGui
{
	#ifndef MAX_FILE_DIALOG_NAME_BUFFER 
	#define MAX_FILE_DIALOG_NAME_BUFFER 1024
	#endif

	#ifndef MAX_PATH_BUFFER_SIZE
	#define MAX_PATH_BUFFER_SIZE 1024
	#endif

	

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
		explicit FileExtentionInfosStruct(const ImVec4& vColor, std::string vIcon = std::string()) : color(vColor), icon(std::move(vIcon)) {}
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

	struct FileDialog
	{
		struct Private
		{
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
			bool m_AnyWindowsHovered = false;
			bool m_CreateDirectoryMode = false;
			bool IsOk = false;

			// events
			bool m_DrivesClicked = false;
			bool m_PathClicked = false;
			bool m_CanWeContinue = true;

#ifdef USE_BOOKMARK
			// bookmarks
			ImGuiListClipper m_BookmarkClipper;
			std::vector<BookmarkStruct> m_Bookmarks;
			bool m_BookmarkPaneShown = false;
#endif

			// flash when select by char
			size_t m_FlashedItem = 0;
			float m_FlashAlpha = 0.0f;
			float m_FlashAlphaAttenInSecs = 1.0f; // fps display dependant
			bool m_OkResultToConfirm = false; // to confim if ok for OverWrite

			// dlg params
			std::string dlg_key;
			std::string dlg_name;
			const char* dlg_filters{};
			std::string dlg_path;
			std::string dlg_defaultFileName;
			std::string dlg_defaultExt;
			ImGuiFileDialogFlags dlg_flags = ImGuiFileDialogFlags_None;
			std::function<void(std::string, IGFDUserDatas, bool*)> dlg_optionsPane;
			float dlg_optionsPaneWidth = 0.0f;
			std::string searchTag;
			IGFDUserDatas dlg_userDatas{};
			size_t dlg_countSelectionMax = 1; // 0 for infinite
			bool dlg_modal = false;

			// detail view
			std::string m_HeaderFileName;
			std::string m_HeaderFileSize;
			std::string m_HeaderFileDate;
			bool m_SortingDirection[3] = { true,true,true }; // true => Descending, false => Ascending
			SortingFieldEnum m_SortingField = SortingFieldEnum::FIELD_FILENAME;

			// buffers
			char InputPathBuffer[MAX_PATH_BUFFER_SIZE] = "";
			char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
			char DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
			char SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#ifdef USE_BOOKMARK
			char BookmarkEditBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#endif

			// will be used in public
			std::string GetCurrentFileName();					// Create File behavior : will always return the content of the field with current filter extention
			std::string GetFilePathName();						// Create File behavior : will always return the content of the field with current filter extention and current path
			bool GetExtentionInfos(const std::string& vFilter, ImVec4* vColor, std::string* vIcon = nullptr);

			// dialog parts
			void DrawHeader();
			void DrawContent();
			bool DrawFooter();

			// widgets components
#ifdef USE_BOOKMARK
			void DrawBookMark();
#endif
			void DrawDirectoryCreation();
			void DrawPathComposer();
			void DrawSearchBar();
			void DrawFileListView(ImVec2 vSize);
			void DrawSidePane(float vHeight);

			// others
			void ResetEvents();
			void SetDefaultFileName(const std::string& vFileName);
			bool SelectDirectory(const FileInfoStruct& vInfos);
			void SelectFileName(const FileInfoStruct& vInfos);
			void RemoveFileNameInSelection(const std::string& vFileName);
			void AddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName);
			void SetPath(const std::string& vPath);
			void FillInfos(FileInfoStruct* vFileInfoStruct);
			void SortFields(SortingFieldEnum vSortingField = SortingFieldEnum::FIELD_NONE, bool vCanChangeOrder = false);
			void ScanDir(const std::string& vPath);
			void SetCurrentDir(const std::string& vPath);
			bool CreateDir(const std::string& vPath);
			std::string ComposeNewPath(std::vector<std::string>::iterator vIter);
			void GetDrives();
			void ParseFilters(const char* vFilters);
			void SetSelectedFilterWithExt(const std::string& vFilter);
			static std::string OptimizeFilenameForSearchOperations(std::string vFileName);

			// filtering
			void ApplyFilteringOnFileList();

#ifdef USE_EXPLORATION_BY_KEYS
			// file localization by input chat // widget flashing
			void LocateByInputKey();
			bool LocateItem_Loop(ImWchar vC);
			void ExploreWithkeys();
			static bool FlashableSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
				bool vFlashing = false, const ImVec2& size = ImVec2(0, 0));
			void StartFlashItem(size_t vIdx);
			bool BeginFlashItem(size_t vIdx);
			void EndFlashItem();
#endif
#ifdef USE_BOOKMARK
			// bookmark
			void DrawBookmarkPane(ImVec2 vSize);
#endif

			// Overwrite Dialog
			bool Confirm_Or_OpenOverWriteFileDialog_IfNeeded(bool vLastAction, ImGuiWindowFlags vFlags);
			bool IsFileExist(const std::string& vFile);
		} m_Private;

		// Init First
		FileDialog();
		
		// standard dialog
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const std::function<void(std::string, IGFDUserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, IGFDUserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);

		// modal dialog
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const std::function<void(std::string, IGFDUserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vDefaultFileName,
			const std::function<void(std::string, IGFDUserDatas, bool*)>& vOptionsPane, const float&  vOptionsPaneWidth = 250.0f,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vPath, const std::string& vDefaultFileName,
			const int& vCountSelectionMax = 1, IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);
		void OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
			const std::string& vFilePathName, const int& vCountSelectionMax = 1,
			IGFDUserDatas vUserDatas = nullptr, ImGuiFileDialogFlags flags = 0);

		// core
		bool Display(const std::string& vKey, ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse,
			ImVec2 vMinSize = ImVec2(0, 0), ImVec2 vMaxSize = ImVec2(FLT_MAX, FLT_MAX));
		void Close(const std::string& vKey);
		void Close();

		bool IsOk();

		bool WasOpenedThisFrame(const std::string& vKey);	// say if the dialog key was already opened this frame
		bool IsOpened(std::string* vCurrentOpenedKey = 0);	// say if the dialog is opened somewhere and can eturn the current opened key dialog	

		std::map<std::string, std::string> GetSelection();	// Open File behavior : will return selection via a map<FileName, FilePathName>
		std::string GetFilePathName();						// Create File behavior : will always return the content of the field with current filter extention and current path
		std::string GetCurrentFileName();					// Create File behavior : will always return the content of the field with current filter extention
		std::string GetCurrentPath();						// will return current path
		std::string GetCurrentFilter();						// get selected filter
		IGFDUserDatas GetUserDatas();							// get user datas send with Open Dialog
		
		void SetExtentionInfos(const std::string& vFilter, const FileExtentionInfosStruct& vInfos);
		void SetExtentionInfos(const std::string& vFilter, const ImVec4& vColor, const std::string& vIcon = "");
		bool GetExtentionInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon = nullptr);
		void ClearExtentionInfos();

#ifdef USE_EXPLORATION_BY_KEYS
		void SetFlashingAttenuationInSeconds(float vAttenValue);
#endif
#ifdef USE_BOOKMARK
		std::string SerializeBookmarks();
		void DeserializeBookmarks(const std::string& vBookmarks);
#endif

	};
}
