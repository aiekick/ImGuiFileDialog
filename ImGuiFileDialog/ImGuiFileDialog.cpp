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

#include "ImGuiFileDialog.h"
#include "imgui.h"

#include <string.h> // strcmpi
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#ifdef WIN32
#define stat _stat
#define stricmp _stricmp
#include <cctype>
#include <dirent.h>
#define PATH_SEP '\\'
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#elif defined(LINUX) or defined(APPLE)
#include <sys/types.h>
#include <dirent.h>
#define PATH_SEP '/'
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace igfd
{
	// for lets you define your button widget
	// if you have like me a speial two color button
	#ifndef IMGUI_PATH_BUTTON
	#define IMGUI_PATH_BUTTON ImGui::Button
	#endif
	#ifndef IMGUI_BUTTON
	#define IMGUI_BUTTON ImGui::Button
	#endif

	// locales
	#ifndef createDirButtonString
	#define createDirButtonString "+"
	#endif
	#ifndef okButtonString
	#define okButtonString "OK"
	#endif
	#ifndef cancelButtonString
	#define cancelButtonString "Cancel"
	#endif
	#ifndef resetButtonString
	#define resetButtonString "R"
	#endif
	#ifndef drivesButtonString
	#define drivesButtonString "Drives"
	#endif
	#ifndef searchString
	#define searchString "Search :"
	#endif
	#ifndef dirEntryString
	#define dirEntryString "[Dir]"
	#endif
	#ifndef linkEntryString
	#define linkEntryString "[Link]"
	#endif
	#ifndef fileEntryString
	#define fileEntryString "[File]"
	#endif
	#ifndef fileNameString
	#define fileNameString "File Name :"
	#endif
	#ifndef dirNameString
	#define dirNameString "Directory Name :"
	#endif
	#ifndef buttonResetSearchString
	#define buttonResetSearchString "Reset search"
	#endif
	#ifndef buttonDriveString
	#define buttonDriveString "Drives"
	#endif
	#ifndef buttonResetPathString
	#define buttonResetPathString "Reset to current directory"
	#endif
	#ifndef buttonCreateDirString
	#define buttonCreateDirString "Create Directory"
	#endif
	#ifndef tableHeaderAscendingIcon
	#define tableHeaderAscendingIcon "A|"
	#endif
	#ifndef tableHeaderDescendingIcon
	#define tableHeaderDescendingIcon "D|"
	#endif
	#ifndef tableHeaderFileNameString
	#define tableHeaderFileNameString "File name"
	#endif
	#ifndef tableHeaderFileSizeString
	#define tableHeaderFileSizeString "Size"
	#endif
	#ifndef tableHeaderFileDateString
	#define tableHeaderFileDateString "Date"
	#endif

	static std::string s_fs_root = std::string(1u, PATH_SEP);

	inline int alphaSort(const struct dirent **a, const struct dirent **b)
	{
		return strcoll((*a)->d_name, (*b)->d_name);
	}

	inline bool replaceString(::std::string& str, const ::std::string& oldStr, const ::std::string& newStr)
	{
		bool found = false;
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != ::std::string::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return found;
	}

	inline std::vector<std::string> splitStringToVector(const ::std::string& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::string> arr;
		if (!text.empty())
		{
			std::string::size_type start = 0;
			std::string::size_type end = text.find(delimiter, start);
			while (end != std::string::npos)
			{
				std::string token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty))
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			arr.push_back(text.substr(start));
		}
		return arr;
	}

	inline std::vector<std::string> GetDrivesList()
	{
		std::vector<std::string> res;

#ifdef WIN32
		DWORD mydrives = 2048;
		char lpBuffer[2048];

		DWORD countChars = GetLogicalDriveStringsA(mydrives, lpBuffer);

		if (countChars > 0)
		{
			std::string var = std::string(lpBuffer, countChars);
			replaceString(var, "\\", "");
			res = splitStringToVector(var, '\0', false);
		}
#endif

		return res;
	}

	inline bool IsDirectoryExist(const std::string& name)
	{
		bool bExists = false;

		if (!name.empty())
		{
			DIR *pDir = nullptr;
			pDir = opendir(name.c_str());
			if (pDir != nullptr)
			{
				bExists = true;
				(void)closedir(pDir);
			}
		}

		return bExists;    // this is not a directory!
	}

	inline bool CreateDirectoryIfNotExist(const std::string& name)
	{
		bool res = false;

		if (!name.empty())
		{
			if (!IsDirectoryExist(name))
			{
				res = true;

#ifdef WIN32
				CreateDirectoryA(name.c_str(), nullptr);
#elif defined(LINUX) or defined(APPLE)
				char buffer[PATH_MAX] = {};
				snprintf(buffer, PATH_MAX, "mkdir -p %s", name.c_str());
				const int dir_err = std::system(buffer);
				if (dir_err == -1)
				{
					std::cout << "Error creating directory " << name << std::endl;
					res = false;
				}
#endif
			}
		}

		return res;
	}

	struct PathStruct
	{
		std::string path;
		std::string name;
		std::string ext;

		bool isOk;

		PathStruct()
		{
			isOk = false;
		}
	};

	inline PathStruct ParsePathFileName(const std::string& vPathFileName)
	{
		PathStruct res;

		if (!vPathFileName.empty())
		{
			std::string pfn = vPathFileName;
			std::string separator(1u, PATH_SEP);
			replaceString(pfn, "\\", separator);
			replaceString(pfn, "/", separator);

			size_t lastSlash = pfn.find_last_of(separator);
			if (lastSlash != std::string::npos)
			{
				res.name = pfn.substr(lastSlash + 1);
				res.path = pfn.substr(0, lastSlash);
				res.isOk = true;
			}

			size_t lastPoint = pfn.find_last_of('.');
			if (lastPoint != std::string::npos)
			{
				if (!res.isOk)
				{
					res.name = pfn;
					res.isOk = true;
				}
				res.ext = pfn.substr(lastPoint + 1);
				replaceString(res.name, "." + res.ext, "");
			}
		}

		return res;
	}

	inline void AppendToBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		std::string st = vStr;
		size_t len = vBufferLen - 1u;
		size_t slen = strlen(vBuffer);

		if (!st.empty() && st != "\n")
		{
			replaceString(st, "\n", "");
			replaceString(st, "\r", "");
		}
		vBuffer[slen] = '\0';
		std::string str = std::string(vBuffer);
		//if (!str.empty()) str += "\n";
		str += vStr;
		if (len > str.size()) len = str.size();
#ifdef MSVC
		strncpy_s(vBuffer, vBufferLen, str.c_str(), len);
#else
		strncpy(vBuffer, str.c_str(), len);
#endif
		vBuffer[len] = '\0';
	}

	inline void ResetBuffer(char* vBuffer)
	{
		vBuffer[0] = '\0';
	}

	char ImGuiFileDialog::FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
	char ImGuiFileDialog::DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
	char ImGuiFileDialog::SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
	int ImGuiFileDialog::FilterIndex = 0;

	ImGuiFileDialog::ImGuiFileDialog()
	{
		m_AnyWindowsHovered = false;
		IsOk = false;
		m_ShowDialog = false;
		m_ShowDrives = false;
		m_CreateDirectoryMode = false;
		dlg_optionsPane = nullptr;
		dlg_optionsPaneWidth = 250;
		dlg_filters = "";
	}

	ImGuiFileDialog::~ImGuiFileDialog() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// STANDARD DIALOG ////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vPath, const std::string& vDefaultFileName,
		const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t& vOptionsPaneWidth,
		const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;
		dlg_path = vPath;
		dlg_defaultFileName = vDefaultFileName;
		dlg_optionsPane = std::move(vOptionsPane);
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = vOptionsPaneWidth;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;

		dlg_defaultExt = "";

		SetPath(m_CurrentPath);
		CheckFilter();

		m_ShowDialog = true;
	}

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vFilePathName,
		const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t& vOptionsPaneWidth,
		const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;

		auto ps = ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			dlg_defaultFileName = vFilePathName;
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = ".";
			dlg_defaultFileName = "";
			dlg_defaultExt = "";
		}

		dlg_optionsPane = std::move(vOptionsPane);
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = vOptionsPaneWidth;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;

		SetPath(m_CurrentPath);
		CheckFilter();

		m_ShowDialog = true;
	}

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vFilePathName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;

		auto ps = ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			dlg_defaultFileName = vFilePathName;
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = ".";
			dlg_defaultFileName = "";
			dlg_defaultExt = "";
		}

		dlg_optionsPane = nullptr;
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = 0;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;

		SetPath(m_CurrentPath);
		CheckFilter();

		m_ShowDialog = true;
	}

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vPath, const std::string& vDefaultFileName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;
		dlg_path = vPath;
		dlg_defaultFileName = vDefaultFileName;
		dlg_optionsPane = nullptr;
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = 0;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;

		dlg_defaultExt = "";

		SetPath(m_CurrentPath);
		CheckFilter();

		m_ShowDialog = true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// STANDARD DIALOG ////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	void ImGuiFileDialog::OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vPath, const std::string& vDefaultFileName,
		const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t& vOptionsPaneWidth,
		const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		OpenDialog(vKey, vName, vFilters,
			vPath, vDefaultFileName,
			vOptionsPane, vOptionsPaneWidth,
			vCountSelectionMax, vUserDatas);
		dlg_modal = true;
	}

	void ImGuiFileDialog::OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vFilePathName,
		const std::function<void(std::string, UserDatas, bool*)>& vOptionsPane, const size_t& vOptionsPaneWidth,
		const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		OpenDialog(vKey, vName, vFilters,
			vFilePathName,
			vOptionsPane, vOptionsPaneWidth,
			vCountSelectionMax, vUserDatas);
		dlg_modal = true;
	}

	void ImGuiFileDialog::OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vFilePathName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		OpenDialog(vKey, vName, vFilters,
			vFilePathName, vCountSelectionMax, vUserDatas);
		dlg_modal = true;
	}

	void ImGuiFileDialog::OpenModal(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vPath, const std::string& vDefaultFileName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		OpenDialog(vKey, vName, vFilters,
			vPath, vDefaultFileName, vCountSelectionMax, vUserDatas);
		dlg_modal = true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// MAIN FUNCTION //////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	bool ImGuiFileDialog::FileDialog(const std::string& vKey, ImGuiWindowFlags vFlags, ImVec2 vMinSize, ImVec2 vMaxSize)
	{
		if (m_ShowDialog && dlg_key == vKey)
		{
			bool res = false;

			std::string name = dlg_name + "##" + dlg_key;

			if (m_Name != name)
			{
				m_FileList.clear();
				m_CurrentPath_Decomposition.clear();
				m_SelectedExt.clear();
			}

			IsOk = false;

			ImGui::SetNextWindowSizeConstraints(vMinSize, vMaxSize);

			bool beg = false;
			if (dlg_modal)
			{
				ImGui::OpenPopup(name.c_str());
				beg = ImGui::BeginPopupModal(name.c_str(), (bool*)nullptr,
					vFlags | ImGuiWindowFlags_NoScrollbar);
			}
			else
			{
				beg = ImGui::Begin(name.c_str(), (bool*)nullptr, vFlags | ImGuiWindowFlags_NoScrollbar);
			}
			if (beg)
			{
				m_Name = name;

				m_AnyWindowsHovered |= ImGui::IsWindowHovered();

				if (dlg_path.empty()) dlg_path = ".";

				if (m_FileList.empty() && !m_ShowDrives)
				{
					replaceString(dlg_defaultFileName, dlg_path, ""); // local path

					if (!dlg_defaultFileName.empty())
					{
						ResetBuffer(FileNameBuffer);
						AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, dlg_defaultFileName);
						//m_SelectedFileName = dlg_defaultFileName;

						if (!dlg_defaultExt.empty())
						{
							m_SelectedExt = dlg_defaultExt;

							ImGuiFileDialog::FilterIndex = 0;
							size_t size = 0;
							const char* p = dlg_filters;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
							while (*p)
							{
								size += strlen(p) + 1;
								p += size;
							}
							int idx = 0;
							auto arr = splitStringToVector(std::string(dlg_filters, size), '\0', false);
							for (auto & it : arr)
							{
								if (m_SelectedExt == it)
								{
									ImGuiFileDialog::FilterIndex = idx;
									break;
								}
								idx++;
							}
						}
					}

					ScanDir(dlg_path);
				}

				if (IMGUI_BUTTON(createDirButtonString))
				{
					if (!m_CreateDirectoryMode)
					{
						m_CreateDirectoryMode = true;
						ResetBuffer(DirectoryNameBuffer);
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(buttonCreateDirString);

				if (m_CreateDirectoryMode)
				{
					ImGui::SameLine();

					ImGui::PushItemWidth(100.0f);
					ImGui::InputText("##DirectoryFileName", DirectoryNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
					ImGui::PopItemWidth();

					ImGui::SameLine();

					if (IMGUI_BUTTON(okButtonString))
					{
						std::string newDir = std::string(DirectoryNameBuffer);
						if (CreateDir(newDir))
						{
							SetPath(m_CurrentPath + PATH_SEP + newDir);
						}

						m_CreateDirectoryMode = false;
					}

					ImGui::SameLine();

					if (IMGUI_BUTTON(cancelButtonString))
					{
						m_CreateDirectoryMode = false;
					}
				}

				ImGui::SameLine();

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				ImGui::SameLine();

				if (IMGUI_BUTTON(resetButtonString))
				{
					SetPath(".");
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(buttonResetPathString);

				bool drivesClick = false;

#ifdef WIN32
				ImGui::SameLine();

				if (IMGUI_BUTTON(drivesButtonString))
				{
					drivesClick = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(buttonDriveString);
#endif

				ImGui::SameLine();

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

				// show current path
				bool pathClick = false;
				if (!m_CurrentPath_Decomposition.empty())
				{
					ImGui::SameLine();
					for (auto itPathDecomp = m_CurrentPath_Decomposition.begin();
						itPathDecomp != m_CurrentPath_Decomposition.end(); ++itPathDecomp)
					{
						if (itPathDecomp != m_CurrentPath_Decomposition.begin())
							ImGui::SameLine();
						if (IMGUI_PATH_BUTTON((*itPathDecomp).c_str()))
						{
							ComposeNewPath(itPathDecomp);
							pathClick = true;
							break;
						}
					}
				}

				// search field
				if (IMGUI_BUTTON(resetButtonString "##ImGuiFileDialogSearchFiled"))
				{
					ResetBuffer(SearchBuffer);
					searchTag.clear();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(buttonResetSearchString);
				ImGui::SameLine();
				ImGui::Text(searchString);
				ImGui::SameLine();
				if (ImGui::InputText("##ImGuiFileDialogSearchFiled", SearchBuffer, MAX_FILE_DIALOG_NAME_BUFFER))
				{
					searchTag = SearchBuffer;
				}

				ImGuiContext& g = *GImGui;
				static float lastBarHeight = 0.0f; // need one frame for calculate filelist size
				ImVec2 size = ImGui::GetContentRegionAvail() - ImVec2((float)dlg_optionsPaneWidth, lastBarHeight);

#ifndef USE_IMGUI_TABLES
				ImGui::BeginChild("##FileDialog_FileList", size);
#else
				static ImGuiTableFlags flags = ImGuiTableFlags_SizingPolicyFixedX | ImGuiTableFlags_RowBg | 
					ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY | 
					ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_ScrollFreezeTopRow 
	#ifndef USE_CUSTOM_SORTING_ICON
					| ImGuiTableFlags_Sortable
	#endif
					;
				if (ImGui::BeginTable("##fileTable", 3, flags, size))
				{
					ImGui::TableSetupColumn(m_HeaderFileName.c_str(), ImGuiTableColumnFlags_WidthStretch, -1, 0);
					ImGui::TableSetupColumn(m_HeaderFileSize.c_str(), ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1, 1);
					ImGui::TableSetupColumn(m_HeaderFileDate.c_str(), ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1, 2);

	#ifndef USE_CUSTOM_SORTING_ICON
					// Sort our data if sort specs have been changed!
					if (const ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
					{
						if (sorts_specs->SpecsChanged && !m_FileList.empty())
						{
							if (sorts_specs->Specs->ColumnUserID == 0)
								SortFields(SortingFieldEnum::FIELD_FILENAME, true);
							else if (sorts_specs->Specs->ColumnUserID == 1)
								SortFields(SortingFieldEnum::FIELD_SIZE, true);
							else if (sorts_specs->Specs->ColumnUserID == 2)
								SortFields(SortingFieldEnum::FIELD_DATE, true);
						}
					}

					ImGui::TableAutoHeaders();
	#else
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    bool columnPressed[3] = {false,false,false};
                    for (int column = 0; column < 3; column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
                        ImGui::PushID(column);
                        ImGui::TableHeader(column_name);
                        ImGui::PopID();
                        if (ImGui::IsItemClicked())
                        {
                            if (column == 0)
                                SortFields(SortingFieldEnum::FIELD_FILENAME, true);
                            else if (column == 1)
                                SortFields(SortingFieldEnum::FIELD_SIZE, true);
                            else if (column == 2)
                                SortFields(SortingFieldEnum::FIELD_DATE, true);
                        }
                    }
	#endif
#endif
					for (auto & it : m_FileList)
					{
						const FileInfoStruct& infos = it;

						bool show = true;

						if (dlg_filters)
						{
							if (infos.type == 'f' && !m_SelectedExt.empty() && (infos.ext != m_SelectedExt && m_SelectedExt != ".*"))
							{
								show = false;
							}
							if (!searchTag.empty() && infos.fileName.find(searchTag) == std::string::npos)
							{
								show = false;
							}
						}
						else // directory mode
						{
							if (infos.type != 'd')
							{
								show = false;
							}
						}

						if (show)
						{
							ImVec4 c;
							std::string icon;
							bool showColor = GetFilterInfos(infos.ext, &c, &icon);
							if (showColor)
								ImGui::PushStyleColor(ImGuiCol_Text, c);

							std::string str = " " + infos.fileName;
							if (infos.type == 'd') str = dirEntryString + str;
							if (infos.type == 'l') str = linkEntryString + str;
							if (infos.type == 'f')
							{
								if (showColor && !icon.empty())
									str = icon + str;
								else
									str = fileEntryString + str;
							}
							bool selected = false;
							if (m_SelectedFileNames.find(infos.fileName) != m_SelectedFileNames.end()) // found
								selected = true;
#ifdef USE_IMGUI_TABLES
							ImGui::TableNextRow();
							if (ImGui::TableSetColumnIndex(0)) // first column
							{
#endif
								if (ImGui::Selectable(str.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick
#ifdef USE_IMGUI_TABLES
									| ImGuiSelectableFlags_SpanAllColumns
#endif
								))
								{
									if (infos.type == 'd')
									{
										if (dlg_filters || ImGui::IsMouseDoubleClicked(0))
										{
											pathClick = SelectDirectory(infos);
										}
										else // directory chooser
										{
											SelectFileName(infos);
										}

										if (showColor)
											ImGui::PopStyleColor();
										break;
									}
									else
									{
										SelectFileName(infos);
									}
								}
#ifdef USE_IMGUI_TABLES
							}
							if (ImGui::TableSetColumnIndex(1)) // second column
							{
								if (infos.type != 'd')
								{
									ImGui::Text("%s ", infos.formatedFileSize.c_str());
								}
							}
							if (ImGui::TableSetColumnIndex(2)) // third column
							{
								ImGui::Text("%s", infos.fileModifDate.c_str());
							}
#endif
							if (showColor)
								ImGui::PopStyleColor();
						}
#ifdef USE_IMGUI_TABLES
					}
					ImGui::EndTable();
#endif
				}

				// changement de repertoire
				if (pathClick)
				{
					SetPath(m_CurrentPath);
				}

				if (drivesClick)
				{
					GetDrives();
				}

#ifndef USE_IMGUI_TABLES
				ImGui::EndChild();
#endif

				float posY = ImGui::GetCursorPos().y; // height of last bar calc

				bool _CanWeContinue = true;

				if (dlg_optionsPane)
				{
					ImGui::SameLine();

					size.x = (float)dlg_optionsPaneWidth;

					ImGui::BeginChild("##FileTypes", size);

					dlg_optionsPane(m_SelectedExt, dlg_userDatas, &_CanWeContinue);

					ImGui::EndChild();
				}

				if (dlg_filters)
					ImGui::Text(fileNameString);
				else // directory chooser
					ImGui::Text(dirNameString);

				ImGui::SameLine();

				float width = ImGui::GetContentRegionAvail().x;
				if (dlg_filters) 
					width -= 120.0f;
				ImGui::PushItemWidth(width);
				ImGui::InputText("##FileName", FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
				ImGui::PopItemWidth();

				if (dlg_filters)
				{
					ImGui::SameLine();

					ImGui::PushItemWidth(100.0f);
					bool comboClick = ImGui::Combo("##Filters", &FilterIndex, dlg_filters) || m_SelectedExt.empty();
					ImGui::PopItemWidth();
					if (comboClick)
					{
						int itemIdx = 0;
						const char* p = dlg_filters;
						while (*p)
						{
							if (FilterIndex == itemIdx)
							{
								m_SelectedExt = std::string(p);
								break;
							}
							p += strlen(p) + 1;
							itemIdx++;
						}
					}
				}

				if (_CanWeContinue)
				{
					if (IMGUI_BUTTON(okButtonString))
					{
						if ('\0' != FileNameBuffer[0])
						{
							IsOk = true;
							res = true;
						}
					}

					ImGui::SameLine();
				}

				if (IMGUI_BUTTON(cancelButtonString))
				{
					IsOk = false;
					res = true;
				}

				lastBarHeight = ImGui::GetCursorPosY() - posY;

				if (dlg_modal)
					ImGui::EndPopup();
			}

			if (!dlg_modal)
				ImGui::End();

			return res;
		}

		return false;
	}

	void ImGuiFileDialog::CloseDialog(const std::string& vKey)
	{
		if (dlg_key == vKey)
		{
			dlg_key.clear();
			m_ShowDialog = false;
		}
	}

	std::string ImGuiFileDialog::GetFilepathName()
	{
		std::string  result = m_CurrentPath;

		if (s_fs_root != result)
		{
			result += PATH_SEP;
		}

		result += GetCurrentFileName();

		return result;
	}

	std::string ImGuiFileDialog::GetCurrentPath()
	{
		return m_CurrentPath;
	}

	std::string ImGuiFileDialog::GetCurrentFileName()
	{
		std::string result = FileNameBuffer;

		size_t lastPoint = result.find_last_of('.');
		if (lastPoint != std::string::npos)
		{
			result = result.substr(0, lastPoint);
		}

		result += m_SelectedExt;

		return result;
	}

	std::string ImGuiFileDialog::GetCurrentFilter()
	{
		return m_SelectedExt;
	}

	UserDatas ImGuiFileDialog::GetUserDatas()
	{
		return dlg_userDatas;
	}

	std::map<std::string, std::string> ImGuiFileDialog::GetSelection()
	{
		std::map<std::string, std::string> res;

		for (auto & it : m_SelectedFileNames)
		{
			std::string  result = m_CurrentPath;

			if (s_fs_root != result)
			{
				result += PATH_SEP;
			}

			result += it;

			res[it] = result;
		}

		return res;
	}

	void ImGuiFileDialog::SetFilterInfos(const std::string& vFilter, FilterInfosStruct vInfos)
	{
		m_FilterInfos[vFilter] = vInfos;
	}

	void ImGuiFileDialog::SetFilterInfos(const std::string& vFilter, ImVec4 vColor, std::string vIcon)
	{
		m_FilterInfos[vFilter] = FilterInfosStruct(vColor, vIcon);
	}

	bool ImGuiFileDialog::GetFilterInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon)
	{
		if (vColor)
		{
			if (m_FilterInfos.find(vFilter) != m_FilterInfos.end()) // found
			{
				*vColor = m_FilterInfos[vFilter].color;
				if (vIcon)
				{
					*vIcon = m_FilterInfos[vFilter].icon;
				}
				return true;
			}
		}
		return false;
	}

	void ImGuiFileDialog::ClearFilterInfos()
	{
		m_FilterInfos.clear();
	}

	bool ImGuiFileDialog::SelectDirectory(const FileInfoStruct& vInfos)
	{
		bool pathClick = false;

		if (vInfos.fileName == "..")
		{
			if (m_CurrentPath_Decomposition.size() > 1)
			{
				ComposeNewPath(m_CurrentPath_Decomposition.end() - 2);
				pathClick = true;
			}
		}
		else
		{
			std::string newPath;

			if (m_ShowDrives)
			{
				newPath = vInfos.fileName + PATH_SEP;
			}
			else
			{
#ifdef LINUX
				if (s_fs_root == m_CurrentPath)
				{
					newPath = m_CurrentPath + vInfos.fileName;
				}
				else
				{
#endif
					newPath = m_CurrentPath + PATH_SEP + vInfos.fileName;
#ifdef LINUX
				}
#endif
			}

			if (IsDirectoryExist(newPath))
			{
				if (m_ShowDrives)
				{
					m_CurrentPath = vInfos.fileName;
					s_fs_root = m_CurrentPath;
				}
				else
				{
					m_CurrentPath = newPath;
				}
				pathClick = true;
			}
		}

		return pathClick;
	}

	void ImGuiFileDialog::SelectFileName(const FileInfoStruct& vInfos)
	{
		if (ImGui::GetIO().KeyCtrl)
		{
			if (dlg_countSelectionMax == 0) // infinite selection
			{
				if (m_SelectedFileNames.find(vInfos.fileName) == m_SelectedFileNames.end()) // not found +> add
				{
					AddFileNameInSelection(vInfos.fileName, true);
				}
				else // found +> remove
				{
					RemoveFileNameInSelection(vInfos.fileName);
				}
			}
			else // selection limited by size
			{
				if (m_SelectedFileNames.size() < dlg_countSelectionMax)
				{
					if (m_SelectedFileNames.find(vInfos.fileName) == m_SelectedFileNames.end()) // not found +> add
					{
						AddFileNameInSelection(vInfos.fileName, true);
					}
					else // found +> remove
					{
						RemoveFileNameInSelection(vInfos.fileName);
					}
				}
			}
		}
		else if (ImGui::GetIO().KeyShift)
		{
			if (dlg_countSelectionMax != 1)
			{
				m_SelectedFileNames.clear();
				// we will iterate filelist and get the last selection after the start selection
				bool startMultiSelection = false;
				std::string fileNameToSelect = vInfos.fileName;
				std::string savedLastSelectedFileName; // for invert selection mode
				for (auto & it : m_FileList)
				{
					const FileInfoStruct& infos = it;

					bool canTake = true;
					if (infos.type == 'f' && !m_SelectedExt.empty() && (infos.ext != m_SelectedExt && m_SelectedExt != ".*")) canTake = false;
					if (!searchTag.empty() && infos.fileName.find(searchTag) == std::string::npos) canTake = false;
					if (canTake) // if not filtered, we will take files who are filtered by the dialog
					{
						if (infos.fileName == m_LastSelectedFileName)
						{
							startMultiSelection = true;
							AddFileNameInSelection(m_LastSelectedFileName, false);
						}
						else if (startMultiSelection)
						{
							if (dlg_countSelectionMax == 0) // infinite selection
							{
								AddFileNameInSelection(infos.fileName, false);
							}
							else // selection limited by size
							{
								if (m_SelectedFileNames.size() < dlg_countSelectionMax)
								{
									AddFileNameInSelection(infos.fileName, false);
								}
								else
								{
									startMultiSelection = false;
									if (!savedLastSelectedFileName.empty())
										m_LastSelectedFileName = savedLastSelectedFileName;
									break;
								}
							}
						}

						if (infos.fileName == fileNameToSelect)
						{
							if (!startMultiSelection) // we are before the last Selected FileName, so we must inverse
							{
								savedLastSelectedFileName = m_LastSelectedFileName;
								m_LastSelectedFileName = fileNameToSelect;
								fileNameToSelect = savedLastSelectedFileName;
								startMultiSelection = true;
								AddFileNameInSelection(m_LastSelectedFileName, false);
							}
							else
							{
								startMultiSelection = false;
								if (!savedLastSelectedFileName.empty())
									m_LastSelectedFileName = savedLastSelectedFileName;
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			m_SelectedFileNames.clear();
			ResetBuffer(FileNameBuffer);
			AddFileNameInSelection(vInfos.fileName, true);
		}
	}

	void ImGuiFileDialog::RemoveFileNameInSelection(const std::string& vFileName)
	{
		m_SelectedFileNames.erase(vFileName);

		if (m_SelectedFileNames.size() == 1)
		{
			snprintf(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", m_SelectedFileNames.size());
		}
	}

	void ImGuiFileDialog::AddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName)
	{
		m_SelectedFileNames.emplace(vFileName);

		if (m_SelectedFileNames.size() == 1)
		{
			snprintf(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", m_SelectedFileNames.size());
		}

		if (vSetLastSelectionFileName)
			m_LastSelectedFileName = vFileName;
	}

	void ImGuiFileDialog::CheckFilter()
	{
		bool found = false;
		int itemIdx = 0;
		const char* p = dlg_filters;
		if (p)
		{
			while (*p)
			{
				if (m_SelectedExt == std::string(p))
				{
					found = true;
					FilterIndex = itemIdx;
					break;
				}
				p += strlen(p) + 1;
				itemIdx++;
			}
		}
		if (!found)
		{
			m_SelectedExt.clear();
			FilterIndex = 0;
		}
	}

	void ImGuiFileDialog::SetPath(const std::string& vPath)
	{
		m_ShowDrives = false;
		m_CurrentPath = vPath;
		m_FileList.clear();
		m_CurrentPath_Decomposition.clear();
		ScanDir(m_CurrentPath);
	}

	static std::string round_n(double vvalue, int n)
	{
		std::stringstream tmp;
		tmp << std::setprecision(n) << std::fixed << vvalue;
		return tmp.str();
	}

	static void FormatFileSize(size_t vByteSize, std::string *vFormat)
	{
		if (vFormat && vByteSize != 0)
		{
			static double lo = 1024;
			static double ko = 1024 * 1024;
			static double mo = 1024 * 1024 * 1024;

			double v = (double)vByteSize;

			if (vByteSize < lo) 
				*vFormat = round_n(v, 0) + " o"; // octet
			else if (vByteSize < ko) 
				*vFormat = round_n(v / lo, 2) + " Ko"; // ko
			else  if (vByteSize < mo) 
				*vFormat = round_n(v / ko, 2) + " Mo"; // Mo 
			else 
				*vFormat = round_n(v / mo, 2) + " Go"; // Go 
		}
	}

	void ImGuiFileDialog::FillInfos(FileInfoStruct *vFileInfoStruct)
	{
		if (vFileInfoStruct && vFileInfoStruct->fileName != "..")
		{
			// _stat struct :
			//dev_t     st_dev;     /* ID of device containing file */
			//ino_t     st_ino;     /* inode number */
			//mode_t    st_mode;    /* protection */
			//nlink_t   st_nlink;   /* number of hard links */
			//uid_t     st_uid;     /* user ID of owner */
			//gid_t     st_gid;     /* group ID of owner */
			//dev_t     st_rdev;    /* device ID (if special file) */
			//off_t     st_size;    /* total size, in bytes */
			//blksize_t st_blksize; /* blocksize for file system I/O */
			//blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
			//time_t    st_atime;   /* time of last access - not sure out of ntfs */
			//time_t    st_mtime;   /* time of last modification - not sure out of ntfs */
			//time_t    st_ctime;   /* time of last status change - not sure out of ntfs */

			std::string fpn;

			if (vFileInfoStruct->type == 'f') // file
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;
			else if (vFileInfoStruct->type == 'l') // link
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;
			else if (vFileInfoStruct->type == 'd') // directory
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;

			struct stat statInfos;
			char timebuf[100];
			int result = stat(fpn.c_str(), &statInfos);
			if (!result)
			{
				if (vFileInfoStruct->type != 'd')
				{
					vFileInfoStruct->fileSize = statInfos.st_size;
					FormatFileSize(vFileInfoStruct->fileSize, 
						&vFileInfoStruct->formatedFileSize);
				}

				size_t len = 0;
#ifdef MSVC
				struct tm _tm;
				errno_t err = localtime_s(&_tm, &statInfos.st_mtime);
				if (!err) len = strftime(timebuf, 99, "%Y/%m/%d ", &_tm);
#else
				struct tm *_tm = localtime(&statInfos.st_mtime);
				if (_tm) len = strftime(timebuf, 99, "%Y/%m/%d ", _tm);
#endif
				if (len)
				{
					vFileInfoStruct->fileModifDate = std::string(timebuf, len);
				}
			}
		}
	}

    void ImGuiFileDialog::SortFields(SortingFieldEnum vSortingField, bool vCanChangeOrder)
    {
		if (vSortingField != SortingFieldEnum::FIELD_NONE)
		{
			m_HeaderFileName = tableHeaderFileNameString;
			m_HeaderFileSize = tableHeaderFileSizeString;
			m_HeaderFileDate = tableHeaderFileDateString;
		}

		if (vSortingField == SortingFieldEnum::FIELD_FILENAME)
		{
			if (vCanChangeOrder && m_SortingField == vSortingField)
				m_SortingDirection[0] = !m_SortingDirection[0];

			if (m_SortingDirection[0])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileName = tableHeaderDescendingIcon + m_HeaderFileName;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type == 'd'); // directory in first
					return (stricmp(a.fileName.c_str(), b.fileName.c_str()) < 0); // sort in insensitive case
				});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileName = tableHeaderAscendingIcon + m_HeaderFileName;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type != 'd'); // directory in last
					return (stricmp(a.fileName.c_str(), b.fileName.c_str()) > 0); // sort in insensitive case
				});
			}
		}
		else if (vSortingField == SortingFieldEnum::FIELD_SIZE)
		{
			if (vCanChangeOrder && m_SortingField == vSortingField)
				m_SortingDirection[1] = !m_SortingDirection[1];

			if (m_SortingDirection[1])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileSize = tableHeaderDescendingIcon + m_HeaderFileSize;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type == 'd'); // directory in first
					return (a.fileSize < b.fileSize); // else
				});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileSize = tableHeaderAscendingIcon + m_HeaderFileSize;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type != 'd'); // directory in last
					return (a.fileSize > b.fileSize); // else
				});
			}
		}
		else if (vSortingField == SortingFieldEnum::FIELD_DATE)
		{
			if (vCanChangeOrder && m_SortingField == vSortingField)
				m_SortingDirection[2] = !m_SortingDirection[2];

			if (m_SortingDirection[2])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileDate = tableHeaderDescendingIcon + m_HeaderFileDate;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type == 'd'); // directory in first
					return (a.fileModifDate < b.fileModifDate); // else
				});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				m_HeaderFileDate = tableHeaderAscendingIcon + m_HeaderFileDate;
#endif
				std::sort(m_FileList.begin(), m_FileList.end(),
					[](const FileInfoStruct & a, const FileInfoStruct & b) -> bool
				{
					if (a.type != b.type) return (a.type != 'd'); // directory in last
					return (a.fileModifDate > b.fileModifDate); // else
				});
			}
		}
		
		if (vSortingField != SortingFieldEnum::FIELD_NONE)
		{
			m_SortingField = vSortingField;
		}
    }

	void ImGuiFileDialog::ScanDir(const std::string& vPath)
	{
		struct dirent **files = nullptr;
		int             i = 0;
		int             n = 0;
		std::string		path = vPath;

		if (m_CurrentPath_Decomposition.empty())
		{
			SetCurrentDir(path);
		}

		if (!m_CurrentPath_Decomposition.empty())
		{
#ifdef WIN32
			if (path == s_fs_root)
			{
				path += PATH_SEP;
			}
#endif
			n = scandir(path.c_str(), &files, nullptr, alphaSort);
			if (n > 0)
			{
				m_FileList.clear();

				for (i = 0; i < n; i++)
				{
					struct dirent *ent = files[i];

					FileInfoStruct infos;

					infos.filePath = path;
					infos.fileName = ent->d_name;
					if (("." != infos.fileName))
					{
						switch (ent->d_type)
						{
						case DT_REG: 
							infos.type = 'f'; break;
						case DT_DIR: 
							infos.type = 'd'; break;
						case DT_LNK: 
							infos.type = 'l'; break;
						}

						if (infos.type == 'f')
						{
							size_t lpt = infos.fileName.find_last_of('.');
							if (lpt != std::string::npos)
							{
								infos.ext = infos.fileName.substr(lpt);
							}
						}

						FillInfos(&infos);
						m_FileList.push_back(infos);
					}
				}

				for (i = 0; i < n; i++)
				{
					free(files[i]);
				}
				free(files);
			}

            SortFields(m_SortingField);
		}
	}

	void ImGuiFileDialog::SetCurrentDir(const std::string& vPath)
	{
		std::string path = vPath;
#ifdef WIN32
		if (s_fs_root == path)
			path += PATH_SEP;
#endif
		DIR  *dir = opendir(path.c_str());
		char  real_path[PATH_MAX];

		if (nullptr == dir)
		{
			path = ".";
			dir = opendir(path.c_str());
		}

		if (nullptr != dir)
		{
#ifdef WIN32
			size_t numchar = GetFullPathNameA(path.c_str(), PATH_MAX - 1, real_path, nullptr);
#elif defined(LINUX) or defined(APPLE)
			char *numchar = realpath(path.c_str(), real_path);
#endif
			if (numchar != 0)
			{
				m_CurrentPath = real_path;
				if (m_CurrentPath[m_CurrentPath.size() - 1] == PATH_SEP)
				{
					m_CurrentPath = m_CurrentPath.substr(0, m_CurrentPath.size() - 1);
				}
				m_CurrentPath_Decomposition = splitStringToVector(m_CurrentPath, PATH_SEP, false);
#if defined(UNIX) // UNIX is LINUX or APPLE
				m_CurrentPath_Decomposition.insert(m_CurrentPath_Decomposition.begin(), std::string(1u, PATH_SEP));
#endif
				if (!m_CurrentPath_Decomposition.empty())
				{
#ifdef WIN32
					s_fs_root = m_CurrentPath_Decomposition[0];
#endif
				}
			}

			closedir(dir);
		}
	}

	bool ImGuiFileDialog::CreateDir(const std::string& vPath)
	{
		bool res = false;

		if (!vPath.empty())
		{
			std::string path = m_CurrentPath + PATH_SEP + vPath;

			res = CreateDirectoryIfNotExist(path);
		}

		return res;
	}

	void ImGuiFileDialog::ComposeNewPath(std::vector<std::string>::iterator vIter)
	{
		m_CurrentPath = "";

		while (true)
		{
			if (!m_CurrentPath.empty())
			{
#ifdef WIN32
				m_CurrentPath = *vIter + PATH_SEP + m_CurrentPath;
#elif defined(LINUX) or defined(APPLE)
				if (*vIter == s_fs_root)
				{
					m_CurrentPath = *vIter + m_CurrentPath;
				}
				else
				{
					m_CurrentPath = *vIter + PATH_SEP + m_CurrentPath;
				}
#endif
			}
			else
			{
				m_CurrentPath = *vIter;
			}

			if (vIter == m_CurrentPath_Decomposition.begin())
			{
#if defined(UNIX) // UNIX is LINUX or APPLE
				if (m_CurrentPath[0] != PATH_SEP)
					m_CurrentPath = PATH_SEP + m_CurrentPath;
#endif
				break;
			}

			vIter--;
		}
	}

	void ImGuiFileDialog::GetDrives()
	{
		auto res = GetDrivesList();
		if (!res.empty())
		{
			m_CurrentPath = "";
			m_CurrentPath_Decomposition.clear();
			m_FileList.clear();
			for (auto & re : res)
			{
				FileInfoStruct infos;
				infos.fileName = re;
				infos.type = 'd';

				if (!infos.fileName.empty())
				{
					m_FileList.push_back(infos);
				}
			}
			m_ShowDrives = true;
		}
	}
}
