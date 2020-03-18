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

#include "imgui.h"

#include <vector>
#include <string>
#include <map>

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <list>

#define MAX_FILE_DIALOG_NAME_BUFFER 1024

#ifndef CUSTOM_IMGUIFILEDIALOG_CONFIG
#include "ImGuiFileDialogConfig.h"
#else
#include CUSTOM_IMGUIFILEDIALOG_CONFIG
#endif

typedef void* UserDatas;

struct FileInfoStruct
{
	char type = ' ';
	std::string filePath;
	std::string fileName;
	std::string ext;
};

class ImGuiFileDialog
{
private:
	std::vector<FileInfoStruct> m_FileList;
	std::map<std::string, ImVec4> m_FilterColor;
	//std::string m_SelectedFileName;
	std::map<std::string, bool> m_SelectedFileNames; // map for have binary search
	std::string m_SelectedExt;
	std::string m_CurrentPath;
	std::vector<std::string> m_CurrentPath_Decomposition;
	std::string m_Name;
	bool m_ShowDialog = false;
	bool m_ShowDrives = false;
	std::string m_LastSelectedFileName; // for shift multi selectio

public:
	static char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
	static char DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
	static char SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
	static int FilterIndex;
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
	UserDatas dlg_userDatas;
	size_t dlg_countSelectionMax = 1; // 0 for infinite
	bool dlg_modal = false;

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

	bool FileDialog(const std::string& vKey, ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse);
	void CloseDialog(const std::string& vKey);
	
	std::string GetFilepathName();
	std::string GetCurrentPath();
	std::string GetCurrentFileName();
	std::string GetCurrentFilter();
	UserDatas GetUserDatas();
	std::map<std::string, std::string> GetSelection(); // return map<FileName, FilePathName>
		
	void SetFilterColor(const std::string& vFilter, ImVec4 vColor);
    bool GetFilterColor(const std::string& vFilter, ImVec4 *vColor);
    void ClearFilterColor();

private:
	bool SelectDirectory(const FileInfoStruct& vInfos);

	void SelectFileName(const FileInfoStruct& vInfos);
	void RemoveFileNameInSelection(const std::string& vFileName);
	void AddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName);

	void CheckFilter();
	
	void SetPath(const std::string& vPath);
	void ScanDir(const std::string& vPath);
	void SetCurrentDir(const std::string& vPath);
	bool CreateDir(const std::string& vPath);
	void ComposeNewPath(std::vector<std::string>::iterator vIter);
	void GetDrives();
};
