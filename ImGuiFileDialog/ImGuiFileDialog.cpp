// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <float.h>
#include <string.h> // stricmp / strcasecmp
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#if defined(__WIN32__) || defined(_WIN32)
#ifndef WIN32
#define WIN32
#endif
#define stat _stat
#define stricmp _stricmp
#include <cctype>
#include <dirent.h>
#define PATH_SEP '\\'
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#define UNIX
#define stricmp strcasecmp
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
	// float comparisons
	#define IS_FLOAT_DIFFERENT(a,b) (fabs((a) - (b)) > FLT_EPSILON)
	#define IS_FLOAT_EQUAL(a,b) (fabs((a) - (b)) < FLT_EPSILON)

	// width of filter combobox
	#ifndef FILTER_COMBO_WIDTH
	#define FILTER_COMBO_WIDTH 150.0f
	#endif
	
	// for lets you define your button widget
	// if you have like me a special bi-color button
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

#ifdef USE_BOOKMARK
	#ifndef bookmarkPaneWith
	#define bookmarkPaneWith 150.0f
	#endif
	#ifndef bookmarksButtonString
	#define bookmarksButtonString "Bookmark"
	#endif
	#ifndef bookmarksButtonHelpString
	#define bookmarksButtonHelpString "Bookmark"
	#endif
	#ifndef addBookmarkButtonString
	#define addBookmarkButtonString "+"
	#endif
	#ifndef removeBookmarkButtonString
	#define removeBookmarkButtonString "-"
	#endif
	#ifndef IMGUI_TOGGLE_BUTTON
	inline bool ToggleButton(const char *vLabel, bool *vToggled)
	{
		bool pressed = false;

		if (vToggled && *vToggled)
		{
			ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			ImVec4 buh = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			ImVec4 bu = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			ImGui::PushStyleColor(ImGuiCol_Button, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
			ImGui::PushStyleColor(ImGuiCol_Text, bua);
		}

		pressed = IMGUI_BUTTON(vLabel);

		if (vToggled && *vToggled)
		{
			ImGui::PopStyleColor(4); //-V112
		}

		if (vToggled && pressed)
			*vToggled = !*vToggled;

		return pressed;
	}
	#define IMGUI_TOGGLE_BUTTON ToggleButton
	#endif
#endif

	static std::string s_fs_root = std::string(1u, PATH_SEP);

	inline int alphaSort(const struct dirent **a, const struct dirent **b)
	{
		return strcoll((*a)->d_name, (*b)->d_name);
	}

	inline bool replaceString(std::string& str, const std::string& oldStr, const std::string& newStr)
	{
		bool found = false;
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != std::string::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return found;
	}

	inline std::vector<std::string> splitStringToVector(const std::string& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::string> arr;
		if (!text.empty())
		{
			std::string::size_type start = 0;
			std::string::size_type end = text.find(delimiter, start);
			while (end != std::string::npos)
			{
				std::string token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty)) //-V728
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			std::string token = text.substr(start);
			if (!token.empty() || (token.empty() && pushEmpty)) //-V728
				arr.push_back(token);
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
			std::string var = std::string(lpBuffer, (size_t)countChars);
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
#elif defined(UNIX)
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

	inline void SetBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		ResetBuffer(vBuffer);
		AppendToBuffer(vBuffer, vBufferLen, vStr);
	}

	char ImGuiFileDialog::InputPathBuffer[MAX_PATH_BUFFER_SIZE] = "";
	char ImGuiFileDialog::FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
	char ImGuiFileDialog::DirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
	char ImGuiFileDialog::SearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#ifdef USE_BOOKMARK
	char ImGuiFileDialog::BookmarkEditBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
#endif
	
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
        dlg_userDatas = 0;
#ifdef USE_BOOKMARK
		m_BookmarkPaneShown = false;
#endif
	}

	ImGuiFileDialog::~ImGuiFileDialog() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// CUSTOM SELECTABLE (Flashing Support) ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_EXPLORATION_BY_KEYS
	bool ImGuiFileDialog::FlashableSelectable(const char* label, bool selected, 
		ImGuiSelectableFlags flags, bool vFlashing, const ImVec2& size_arg)
	{
		using namespace ImGui;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns) // FIXME-OPT: Avoid if vertically clipped.
			PushColumnsBackground();

		// Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
		ImGuiID id = window->GetID(label);
		ImVec2 label_size = CalcTextSize(label, NULL, true);
		ImVec2 size(
			IS_FLOAT_DIFFERENT(size_arg.x, 0.0f) ? size_arg.x : label_size.x, 
			IS_FLOAT_DIFFERENT(size_arg.y, 0.0f) ? size_arg.y : label_size.y
		);
		ImVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;
		ItemSize(size, 0.0f);

		// Fill horizontal space
		const float min_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? window->ContentRegionRect.Min.x : pos.x;
		const float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? window->ContentRegionRect.Max.x : GetContentRegionMaxAbs().x;
		if (IS_FLOAT_DIFFERENT(size_arg.x, 0.0f) || (flags & ImGuiSelectableFlags_SpanAvailWidth))
			size.x = ImMax(label_size.x, max_x - min_x);

		// Text stays at the submission position, but bounding box may be extended on both sides
		const ImVec2 text_min = pos;
		const ImVec2 text_max(min_x + size.x, pos.y + size.y);

		// Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
		ImRect bb_enlarged(min_x, pos.y, text_max.x, text_max.y);
		const float spacing_x = style.ItemSpacing.x;
		const float spacing_y = style.ItemSpacing.y;
		const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
		const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
		bb_enlarged.Min.x -= spacing_L;
		bb_enlarged.Min.y -= spacing_U;
		bb_enlarged.Max.x += (spacing_x - spacing_L);
		bb_enlarged.Max.y += (spacing_y - spacing_U);
		//if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb_align.Min, bb_align.Max, IM_COL32(255, 0, 0, 255)); }
		//if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb_enlarged.Min, bb_enlarged.Max, IM_COL32(0, 255, 0, 255)); }

		bool item_add;
		if (flags & ImGuiSelectableFlags_Disabled)
		{
			ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
			window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
			item_add = ItemAdd(bb_enlarged, id);
			window->DC.ItemFlags = backup_item_flags;
		}
		else
		{
			item_add = ItemAdd(bb_enlarged, id);
		}
		if (!item_add)
		{
			if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
				PopColumnsBackground();
			return false;
		}

		// We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
		ImGuiButtonFlags button_flags = 0;
		if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
		if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
		if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
		if (flags & ImGuiSelectableFlags_Disabled) { button_flags |= ImGuiButtonFlags_Disabled; }
		if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
		if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

		if (flags & ImGuiSelectableFlags_Disabled)
			selected = false;

		const bool was_selected = selected;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb_enlarged, id, &hovered, &held, button_flags);

		// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
		if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
		{
			if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
			{
				g.NavDisableHighlight = true;
				SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent);
			}
		}
		if (pressed)
			MarkItemEdited(id);

		if (flags & ImGuiSelectableFlags_AllowItemOverlap)
			SetItemAllowOverlap();

		// In this branch, Selectable() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		if ((held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld)) || vFlashing)
			hovered = true;
		if (hovered || selected)
		{
			const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
			RenderFrame(bb_enlarged.Min, bb_enlarged.Max, col, false, 0.0f);
			RenderNavHighlight(bb_enlarged, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
		}

		if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
			PopColumnsBackground();

		if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
		RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb_enlarged);
		if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

		// Automatically close popups
		if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
			CloseCurrentPopup();

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
		return pressed;
	}
#endif

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
		ParseFilters(dlg_filters);
		dlg_path = vPath;
		SetDefaultFileName(vDefaultFileName);
		dlg_optionsPane = std::move(vOptionsPane);
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = vOptionsPaneWidth;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		dlg_defaultExt.clear();

		SetPath(m_CurrentPath);

		m_ShowDialog = true;
#ifdef USE_BOOKMARK
		m_BookmarkPaneShown = false;
#endif
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
		ParseFilters(dlg_filters);

		auto ps = ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			SetDefaultFileName(vFilePathName);
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = "."; 
			SetDefaultFileName("");
			dlg_defaultExt.clear();
		}

		dlg_optionsPane = std::move(vOptionsPane);
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = vOptionsPaneWidth;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		SetSelectedFilterWithExt(dlg_defaultExt);
		SetPath(m_CurrentPath);

		m_ShowDialog = true;
#ifdef USE_BOOKMARK
		m_BookmarkPaneShown = false;
#endif
	}

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vFilePathName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;
		ParseFilters(dlg_filters);

		auto ps = ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			SetDefaultFileName(vFilePathName);
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = ".";
			SetDefaultFileName("");
			dlg_defaultExt.clear();
		}

		dlg_optionsPane = nullptr;
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = 0;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		SetSelectedFilterWithExt(dlg_defaultExt);
		SetPath(m_CurrentPath);

		m_ShowDialog = true;
#ifdef USE_BOOKMARK
		m_BookmarkPaneShown = false;
#endif
	}

	void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
		const std::string& vPath, const std::string& vDefaultFileName, const int& vCountSelectionMax, UserDatas vUserDatas)
	{
		if (m_ShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_name = std::string(vName);
		dlg_filters = vFilters;
		ParseFilters(dlg_filters);
		dlg_path = vPath;
		SetDefaultFileName(vDefaultFileName);
		dlg_optionsPane = nullptr;
		dlg_userDatas = vUserDatas;
		dlg_optionsPaneWidth = 0;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		dlg_defaultExt.clear();

		SetPath(m_CurrentPath);

		m_ShowDialog = true;
#ifdef USE_BOOKMARK
		m_BookmarkPaneShown = false;
#endif
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
				m_Name = name; //-V820

				m_AnyWindowsHovered |= ImGui::IsWindowHovered();

				if (dlg_path.empty()) dlg_path = ".";

				if (m_SelectedFilter.empty())
				{
					if (!m_Filters.empty())
					{
						m_SelectedFilter = *m_Filters.begin();
					}
				}

				if (m_FileList.empty() && !m_ShowDrives)
				{
					replaceString(dlg_defaultFileName, dlg_path, ""); // local path

					if (!dlg_defaultFileName.empty())
					{
						SetDefaultFileName(dlg_defaultFileName);
						SetSelectedFilterWithExt(dlg_defaultExt);
					}
					
					ScanDir(dlg_path);
				}

#ifdef USE_BOOKMARK
				IMGUI_TOGGLE_BUTTON(bookmarksButtonString, &m_BookmarkPaneShown);

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(bookmarksButtonHelpString);

				ImGui::SameLine();
#endif

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

                    if (m_InputPathActivated)
                    {
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
						ImGui::InputText("##pathedition", InputPathBuffer, MAX_PATH_BUFFER_SIZE);
                        ImGui::PopItemWidth();
                    }
                    else
                    {
						int _id = 0;
						for (auto itPathDecomp = m_CurrentPath_Decomposition.begin();
                             itPathDecomp != m_CurrentPath_Decomposition.end(); ++itPathDecomp)
                        {
                            if (itPathDecomp != m_CurrentPath_Decomposition.begin())
                                ImGui::SameLine();
                            ImGui::PushID(_id++);
                            bool click = IMGUI_PATH_BUTTON((*itPathDecomp).c_str());
                            ImGui::PopID();
                            if (click)
                            {
								m_CurrentPath = ComposeNewPath(itPathDecomp);
                                pathClick = true;
                                break;
                            }
                            // activate input for path
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                            {
								SetBuffer(InputPathBuffer, MAX_PATH_BUFFER_SIZE, ComposeNewPath(itPathDecomp));
								m_InputPathActivated = true;
								break;
                            }
                        }
                    }
                }

				// search field
				if (IMGUI_BUTTON(resetButtonString "##ImGuiFileDialogSearchFiled"))
				{
					ResetBuffer(SearchBuffer);
					searchTag.clear();
					ApplyFilteringOnFileList();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(buttonResetSearchString);
				ImGui::SameLine();
				ImGui::Text(searchString);
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				bool edited = ImGui::InputText("##ImGuiFileDialogSearchFiled", SearchBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
				ImGui::PopItemWidth();
				if (edited)
				{
					searchTag = SearchBuffer;
                    ApplyFilteringOnFileList();
				}
				static float lastBarHeight = 0.0f; // need one frame for calculate filelist size

#ifdef USE_BOOKMARK
				if (m_BookmarkPaneShown)
				{
					ImVec2 size = ImVec2(bookmarkPaneWith, ImGui::GetContentRegionAvail().y - lastBarHeight);
					DrawBookmarkPane(size);
					ImGui::SameLine();
				}
#endif

				ImVec2 size = ImGui::GetContentRegionAvail() - ImVec2((float)dlg_optionsPaneWidth, lastBarHeight);
#ifndef USE_IMGUI_TABLES
				ImGui::BeginChild("##FileDialog_FileList", size);
#else
				static ImGuiTableFlags flags = ImGuiTableFlags_SizingPolicyFixedX | ImGuiTableFlags_RowBg | 
					ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY | 
					ImGuiTableFlags_NoHostExtendY 
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
                            else //if (column == 2) => alwasy true for the moment, to uncomment if we add a fourth column
                                SortFields(SortingFieldEnum::FIELD_DATE, true);
                        }
                    }
	#endif
#endif
				int countRows = (int)m_FilteredFileList.size();
                ImGuiListClipper clipper(countRows, ImGui::GetTextLineHeightWithSpacing());
                while(clipper.Step())
                {
                    for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    {
                        const FileInfoStruct& infos = m_FilteredFileList[i];

                        ImVec4 c;
                        std::string icon;
                        bool showColor = GetExtentionInfos(infos.ext, &c, &icon);
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
                        ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
    #ifdef USE_IMGUI_TABLES
                        selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SpanAvailWidth;
    #endif
                        bool _selectablePressed = false;
    #ifdef USE_EXPLORATION_BY_KEYS
                        bool flashed = BeginFlashItem(i);
                        _selectablePressed = FlashableSelectable(str.c_str(), selected, selectableFlags, flashed);
                        if (flashed)
                            EndFlashItem();
    #else
                        _selectablePressed = ImGui::Selectable(str.c_str(), selected, selectableFlags);
    #endif
                        if (_selectablePressed)
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
							ImGui::Text("%s ", infos.formatedFileSize.c_str()); //-V111
						}
					}
					if (ImGui::TableSetColumnIndex(2)) // third column
					{
						ImGui::Text("%s", infos.fileModifDate.c_str()); //-V111
					}
    #endif
					if (showColor)
						ImGui::PopStyleColor();

                    }
                }
                clipper.End();

                if (m_InputPathActivated)
                {
					auto gio = ImGui::GetIO();
                    if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Enter]))
                    {
						SetPath(std::string(InputPathBuffer));
                        m_InputPathActivated = false;
                    }
					if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Escape]))
					{
						m_InputPathActivated = false;
					}
                }
#ifdef USE_EXPLORATION_BY_KEYS
                else
                {
                        LocateByInputKey();
                        ExploreWithkeys();
                }
#endif
#ifdef USE_IMGUI_TABLES
				ImGui::EndTable();
				}
#endif
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

					dlg_optionsPane(m_SelectedFilter.filter, dlg_userDatas, &_CanWeContinue);

					ImGui::EndChild();
				}

				if (dlg_filters)
					ImGui::Text(fileNameString);
				else // directory chooser
					ImGui::Text(dirNameString);

				ImGui::SameLine();

				float width = ImGui::GetContentRegionAvail().x;
				if (dlg_filters) 
					width -= FILTER_COMBO_WIDTH;
				ImGui::PushItemWidth(width);
				ImGui::InputText("##FileName", FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
				ImGui::PopItemWidth();

				if (dlg_filters)
				{
					ImGui::SameLine();

					bool needToApllyNewFilter = false;

					ImGui::PushItemWidth(FILTER_COMBO_WIDTH);
					if (ImGui::BeginCombo("##Filters", m_SelectedFilter.filter.c_str(), ImGuiComboFlags_None))
					{
						intptr_t i = 0;
						for (auto filter : m_Filters)
						{
							const bool item_selected = (filter.filter == m_SelectedFilter.filter);
							ImGui::PushID((void*)(intptr_t)i++);
							if (ImGui::Selectable(filter.filter.c_str(), item_selected))
							{
								m_SelectedFilter = filter;
								needToApllyNewFilter = true;
							}
							ImGui::PopID();
						}
						
						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();

					if (needToApllyNewFilter)
					{
						SetPath(m_CurrentPath);
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

	std::string ImGuiFileDialog::GetFilePathName()
	{
		std::string  result = m_CurrentPath;

#ifdef UNIX
		if (s_fs_root != result)
#endif
			result += PATH_SEP;

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

		// if not a collection we can replace the filter by thee extention we want
		if (m_SelectedFilter.collectionfilters.empty())
		{
			size_t lastPoint = result.find_last_of('.');
			if (lastPoint != std::string::npos)
			{
				result = result.substr(0, lastPoint);
			}

			result += m_SelectedFilter.filter;
		}

		return result;
	}

	std::string ImGuiFileDialog::GetCurrentFilter()
	{
		return m_SelectedFilter.filter;
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
			std::string result = m_CurrentPath;

#ifdef UNIX
			if (s_fs_root != result)
#endif
				result += PATH_SEP;

			result += it;

			res[it] = result;
		}

		return res;
	}

	void ImGuiFileDialog::SetExtentionInfos(const std::string& vFilter, const FileExtentionInfosStruct& vInfos)
	{
		m_FileExtentionInfos[vFilter] = vInfos;
	}

	void ImGuiFileDialog::SetExtentionInfos(const std::string& vFilter, const ImVec4& vColor, const std::string& vIcon)
	{
		m_FileExtentionInfos[vFilter] = FileExtentionInfosStruct(vColor, vIcon);
	}

	bool ImGuiFileDialog::GetExtentionInfos(const std::string& vFilter, ImVec4 *vColor, std::string *vIcon)
	{
		if (vColor)
		{
			if (m_FileExtentionInfos.find(vFilter) != m_FileExtentionInfos.end()) // found
			{
				*vColor = m_FileExtentionInfos[vFilter].color;
				if (vIcon)
				{
					*vIcon = m_FileExtentionInfos[vFilter].icon;
				}
				return true;
			}
		}
		return false;
	}

	void ImGuiFileDialog::ClearExtentionInfos()
	{
		m_FileExtentionInfos.clear();
	}

	void ImGuiFileDialog::SetDefaultFileName(const std::string& vFileName)
	{
		dlg_defaultFileName = vFileName;
		SetBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vFileName);
	}

	bool ImGuiFileDialog::SelectDirectory(const FileInfoStruct& vInfos)
	{
		bool pathClick = false;

		if (vInfos.fileName == "..")
		{
			if (m_CurrentPath_Decomposition.size() > 1)
			{
				m_CurrentPath = ComposeNewPath(m_CurrentPath_Decomposition.end() - 2);
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
#ifdef __linux__
				if (s_fs_root == m_CurrentPath)
					newPath = m_CurrentPath + vInfos.fileName;
				else
#endif
					newPath = m_CurrentPath + PATH_SEP + vInfos.fileName;
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
					m_CurrentPath = newPath; //-V820
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
			
			if (v < lo) 
				*vFormat = round_n(v, 0) + " o"; // octet
			else if (v < ko) 
				*vFormat = round_n(v / lo, 2) + " Ko"; // ko
			else  if (v < mo) 
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
					vFileInfoStruct->fileSize = (size_t)statInfos.st_size;
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

		ApplyFilteringOnFileList();
    }

	void ImGuiFileDialog::ScanDir(const std::string& vPath)
	{
		struct dirent **files = nullptr;
		int          i = 0;
		int          n = 0;
		std::string		path = vPath;

		if (m_CurrentPath_Decomposition.empty())
		{
			SetCurrentDir(path);
		}

		if (!m_CurrentPath_Decomposition.empty())
		{
#ifdef WIN32
			if (path == s_fs_root)
				path += PATH_SEP;
#endif
			n = scandir(path.c_str(), &files, nullptr, alphaSort);

            m_FileList.clear();

            if (n > 0)
			{
				for (i = 0; i < n; i++)
				{
					struct dirent *ent = files[i];

					FileInfoStruct infos;

					infos.filePath = path;
					infos.fileName = ent->d_name;
					infos.fileName_optimized = OptimizeFilenameForSearchOperations(infos.fileName);

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

						if (infos.type == 'f' || 
							infos.type == 'l') // link can have the same extention of a file
						{
							size_t lpt = infos.fileName.find_last_of('.');
							if (lpt != std::string::npos)
							{
								infos.ext = infos.fileName.substr(lpt);
							}

							if (dlg_filters)
							{
								// check if current file extention is covered by current filter
								// we do that here, for avoid doing taht during filelist display
								// for better fps
								if (!m_SelectedFilter.empty() && // selected filter exist
									(!m_SelectedFilter.filterExist(infos.ext) && // filter not found
										m_SelectedFilter.filter != ".*"))
								{
									continue;
								}
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
#elif defined(UNIX) // UNIX is LINUX or APPLE
			char *numchar = realpath(path.c_str(), real_path);
#endif
			if (numchar != 0)
			{
				m_CurrentPath = real_path;
				if (m_CurrentPath[m_CurrentPath.size() - 1] == PATH_SEP)
				{
					m_CurrentPath = m_CurrentPath.substr(0, m_CurrentPath.size() - 1);
				}
				SetBuffer(InputPathBuffer, MAX_PATH_BUFFER_SIZE, m_CurrentPath);
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

	std::string ImGuiFileDialog::ComposeNewPath(std::vector<std::string>::iterator vIter)
	{
		std::string res;

		while (true)
		{
			if (!res.empty())
			{
#ifdef WIN32
				res = *vIter + PATH_SEP + res;
#elif defined(UNIX) // UNIX is LINUX or APPLE
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
				res = *vIter;
			}

			if (vIter == m_CurrentPath_Decomposition.begin())
			{
#if defined(UNIX) // UNIX is LINUX or APPLE
				if (m_CurrentPath[0] != PATH_SEP)
					m_CurrentPath = PATH_SEP + m_CurrentPath;
#endif
				break;
			}

			--vIter;
		}

		return res;
	}

	void ImGuiFileDialog::GetDrives()
	{
		auto res = GetDrivesList();
		if (!res.empty())
		{
			m_CurrentPath.clear();
			m_CurrentPath_Decomposition.clear();
			m_FileList.clear();
			for (auto & re : res)
			{
				FileInfoStruct infos;
				infos.fileName = re;
				infos.fileName_optimized = OptimizeFilenameForSearchOperations(re);
				infos.type = 'd';

				if (!infos.fileName.empty())
				{
					m_FileList.push_back(infos);
				}
			}
			m_ShowDrives = true;
            ApplyFilteringOnFileList();
		}
	}

	void ImGuiFileDialog::ParseFilters(const char *vFilters)
	{
		m_Filters.clear();

		if (vFilters)
		{
			std::string fullStr = vFilters;

			// ".*,.cpp,.h,.hpp"
			// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md"

			bool currentFilterFound = false;

			size_t nan = std::string::npos;
			size_t p = 0, lp = 0;
			while ((p = fullStr.find_first_of("{,", p)) != nan)
			{
				FilterInfosStruct infos;

				if (fullStr[p] == '{') // {
				{
					infos.filter = fullStr.substr(lp, p - lp);
					p++;
					lp = fullStr.find('}', p);
					if (lp != nan)
					{
						std::string fs = fullStr.substr(p, lp - p);
						auto arr = splitStringToVector(fs, ',', false);
						for (auto a : arr)
						{
							infos.collectionfilters.emplace(a);
						}
					}
					p = lp + 1;
				}
				else // ,
				{
					infos.filter = fullStr.substr(lp, p - lp);
					p++;
				}

				if (!currentFilterFound && m_SelectedFilter.filter == infos.filter)
				{
					currentFilterFound = true;
					m_SelectedFilter = infos;
				}

				lp = p;
				if (!infos.empty())
					m_Filters.emplace_back(infos);
			}

			std::string token = fullStr.substr(lp);
			if (!token.empty())
			{
				FilterInfosStruct infos;
				infos.filter = token;
				m_Filters.emplace_back(infos);
			}

			if (!currentFilterFound)
				if (!m_Filters.empty())
					m_SelectedFilter = *m_Filters.begin();
		}
	}

	void ImGuiFileDialog::SetSelectedFilterWithExt(const std::string& vFilter)
	{
		if (!m_Filters.empty())
		{
			if (!vFilter.empty())
			{
				// std::map<std::string, FilterInfosStruct>
				for (auto infos : m_Filters)
				{
					if (vFilter == infos.filter)
					{
						m_SelectedFilter = infos;
					}
					else
					{
						// maybe this ext is in an extention so we will 
						// explore the collections is they are existing
						for (auto filter : infos.collectionfilters)
						{
							if (vFilter == filter)
							{
								m_SelectedFilter = infos;
							}
						}
					}
				}
			}

			if (m_SelectedFilter.empty())
				m_SelectedFilter = *m_Filters.begin();
		}
	}
	
	std::string ImGuiFileDialog::OptimizeFilenameForSearchOperations(std::string vFileName)
	{
		// convert to lower case
		for (char & c : vFileName)
			c = (char)std::tolower(c);
		return vFileName;
	}

    void ImGuiFileDialog::ApplyFilteringOnFileList()
    {
        m_FilteredFileList.clear();

        for (auto &it : m_FileList)
        {
            const FileInfoStruct &infos = it;

            bool show = true;

            // if search tag
            if (!searchTag.empty() &&
                infos.fileName_optimized.find(searchTag) == std::string::npos && // first try wihtout case and accents
                infos.fileName.find(searchTag) == std::string::npos) // second if searched with case and accents
            {
                show = false;
            }

            if (!dlg_filters && infos.type != 'd') // directory mode
            {
                show = false;
            }

            if (show)
            {
                m_FilteredFileList.push_back(infos);
            }
        }
    }

#ifdef USE_EXPLORATION_BY_KEYS
    ////////////////////////////////////////////////////////////////////////////////////////////////
	//// LOCATE / EXPLORE WITH KEYS ////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////

	static size_t locateFileByInputChar_lastFileIdx = 0;
	static ImWchar locateFileByInputChar_lastChar = 0;
	static int locateFileByInputChar_InputQueueCharactersSize = 0;
	static bool locateFileByInputChar_lastFound = false;

	bool ImGuiFileDialog::LocateItem_Loop(ImWchar vC)
	{
		bool found = false;

		for (size_t i = locateFileByInputChar_lastFileIdx; i < m_FilteredFileList.size(); i++)
		{
			if (m_FilteredFileList[i].fileName_optimized[0] == vC || // lower case search
				m_FilteredFileList[i].fileName[0] == vC) // maybe upper case search
			{
				//float p = ((float)i) * ImGui::GetTextLineHeightWithSpacing();
				float p = (float)((double)i / (double)m_FilteredFileList.size()) * ImGui::GetScrollMaxY();
				ImGui::SetScrollY(p);
				locateFileByInputChar_lastFound = true;
				locateFileByInputChar_lastFileIdx = i;
				StartFlashItem(locateFileByInputChar_lastFileIdx);

				auto infos = &m_FilteredFileList[locateFileByInputChar_lastFileIdx];

				if (infos->type == 'd')
				{
					if (!dlg_filters) // directory chooser
					{
						SelectFileName(*infos);
					}
				}
				else
				{
					SelectFileName(*infos);
				}

				found = true;

				break;
			}
		}

		return found;
	}

	void ImGuiFileDialog::LocateByInputKey()
	{
		ImGuiContext& g = *GImGui;
		if (!g.ActiveId && !m_FilteredFileList.empty())
		{
			auto &queueChar = ImGui::GetIO().InputQueueCharacters;

			// point by char
			if (!queueChar.empty())
			{
				ImWchar c = queueChar.back();
				if (locateFileByInputChar_InputQueueCharactersSize != queueChar.size())
				{
					if (c == locateFileByInputChar_lastChar) // next file starting with same char until
					{
						if (locateFileByInputChar_lastFileIdx < m_FilteredFileList.size() - 1)
							locateFileByInputChar_lastFileIdx++;
						else
							locateFileByInputChar_lastFileIdx = 0;
					}

					if (!LocateItem_Loop(c))
					{
						// not found, loop again from 0 this time
						locateFileByInputChar_lastFileIdx = 0;
						LocateItem_Loop(c);
					}

					locateFileByInputChar_lastChar = c;
				}
			}

			locateFileByInputChar_InputQueueCharactersSize = queueChar.size();
		}
	}

	void ImGuiFileDialog::ExploreWithkeys()
	{
        ImGuiContext& g = *GImGui;
        if (!g.ActiveId && !m_FilteredFileList.empty())
        {
            // explore
            bool exploreByKey = false;
            bool enterInDirectory = false;
            bool exitDirectory = false;
            if (ImGui::IsKeyPressed(IGFD_KEY_UP))
            {
                exploreByKey = true;
                if (locateFileByInputChar_lastFileIdx > 0)
                    locateFileByInputChar_lastFileIdx--;
            }
            else if (ImGui::IsKeyPressed(IGFD_KEY_DOWN))
            {
                exploreByKey = true;
                if (locateFileByInputChar_lastFileIdx < m_FilteredFileList.size() - 1)
                    locateFileByInputChar_lastFileIdx++;
            }
            else if (ImGui::IsKeyReleased(IGFD_KEY_ENTER))
            {
                exploreByKey = true;
                enterInDirectory = true;
            }
            else if (ImGui::IsKeyReleased(IGFD_KEY_BACKSPACE))
            {
                exploreByKey = true;
                exitDirectory = true;
            }

            if (exploreByKey)
            {
				//float totalHeight = m_FilteredFileList.size() * ImGui::GetTextLineHeightWithSpacing();
				float p = (float)((double)locateFileByInputChar_lastFileIdx / (double)m_FilteredFileList.size()) * ImGui::GetScrollMaxY();// seems not udpated in tables version outside tables
				//float p = ((float)locateFileByInputChar_lastFileIdx) * ImGui::GetTextLineHeightWithSpacing();
				ImGui::SetScrollY(p);
                StartFlashItem(locateFileByInputChar_lastFileIdx);
				
                auto infos = &m_FilteredFileList[locateFileByInputChar_lastFileIdx];

                if (infos->type == 'd')
                {
                    if (dlg_filters || enterInDirectory)
                    {
                        if (enterInDirectory)
                        {
                            if (SelectDirectory(*infos))
                            {
                                // changement de repertoire
                                SetPath(m_CurrentPath);
                                if (locateFileByInputChar_lastFileIdx > m_FilteredFileList.size() - 1)
                                {
                                    locateFileByInputChar_lastFileIdx = 0;
                                }
                            }
                        }
                    }
                    else // directory chooser
                    {
                        SelectFileName(*infos);
                    }
                }
                else
                {
                    SelectFileName(*infos);
                }

                if (exitDirectory)
                {
                    FileInfoStruct nfo;
                    nfo.fileName = "..";

                    if (SelectDirectory(nfo))
                    {
                        // changement de repertoire
                        SetPath(m_CurrentPath);
                        if (locateFileByInputChar_lastFileIdx > m_FilteredFileList.size() - 1)
                        {
                            locateFileByInputChar_lastFileIdx = 0;
                        }
                    }
#ifdef WIN32
                    else
                    {
                        if (m_CurrentPath_Decomposition.size() == 1)
                        {
                            GetDrives(); // display drives
                        }
                    }
#endif
                }
            }
        }
	}

	void ImGuiFileDialog::StartFlashItem(size_t vIdx)
	{
		m_FlashAlpha = 1.0f;
		m_FlashedItem = vIdx;
	}

	bool ImGuiFileDialog::BeginFlashItem(size_t vIdx)
	{
		bool res = false;

		if (m_FlashedItem == vIdx && 
			std::abs(m_FlashAlpha - 0.0f) > 0.00001f)
		{
			m_FlashAlpha -= m_FlashAlphaAttenInSecs * ImGui::GetIO().DeltaTime;
			if (m_FlashAlpha < 0.0f) m_FlashAlpha = 0.0f;

			ImVec4 hov = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			hov.w = m_FlashAlpha;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hov);
			res = true;
		}

		return res;
	}

	void ImGuiFileDialog::EndFlashItem()
	{
		ImGui::PopStyleColor();
	}

	void ImGuiFileDialog::SetFlashingAttenuationInSeconds(float vAttenValue)
	{
		m_FlashAlphaAttenInSecs = 1.0f / ImMax(vAttenValue,0.01f);
	}
#endif

#ifdef USE_BOOKMARK
	void ImGuiFileDialog::DrawBookmarkPane(ImVec2 vSize)
	{
		ImGui::BeginChild("##bookmarkpane", vSize);
		if (IMGUI_BUTTON(addBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
		{
			if (!m_CurrentPath_Decomposition.empty())
			{
				BookmarkStruct bookmark;
				bookmark.name = m_CurrentPath_Decomposition.back();
				bookmark.path = m_CurrentPath;
				m_Bookmarks.push_back(bookmark);
			}
		}
		static int selectedBookmarkForEdition = -1;
		if (selectedBookmarkForEdition >= 0 &&
			selectedBookmarkForEdition < (int)m_Bookmarks.size())
		{
			ImGui::SameLine();
			if (IMGUI_BUTTON(removeBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
			{
				m_Bookmarks.erase(m_Bookmarks.begin() + selectedBookmarkForEdition);
				if (selectedBookmarkForEdition == (int)m_Bookmarks.size())
					selectedBookmarkForEdition--;
			}
			if (selectedBookmarkForEdition >= 0)
			{
				ImGui::PushItemWidth(bookmarkPaneWith);
				if (ImGui::InputText("##ImGuiFileDialogBookmarkEdit", BookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER))
				{
					m_Bookmarks[selectedBookmarkForEdition].name = std::string(BookmarkEditBuffer);
				}
				ImGui::PopItemWidth();
			}
		}
		ImGui::Separator();
		int countRows = (int)m_Bookmarks.size();
		ImGuiListClipper clipper(countRows, ImGui::GetTextLineHeightWithSpacing());
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const BookmarkStruct& bookmark = m_Bookmarks[i];
			ImGui::PushID(i);
			if (ImGui::Selectable(bookmark.name.c_str(), selectedBookmarkForEdition == i, ImGuiSelectableFlags_AllowDoubleClick) ||
				(selectedBookmarkForEdition == -1 && bookmark.path == m_CurrentPath)) // select if path is current
			{
				selectedBookmarkForEdition = i;
				ResetBuffer(BookmarkEditBuffer);
				AppendToBuffer(BookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER, bookmark.name);

				if (ImGui::IsMouseDoubleClicked(0)) // apply path
				{
					SetPath(bookmark.path);
				}
			}
			ImGui::PopID();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(bookmark.path.c_str());
		}
		clipper.End();
		ImGui::EndChild();
	}

	std::string ImGuiFileDialog::SerializeBookmarks()
	{
		std::string res;

		size_t idx = 0;
		for (auto & it : m_Bookmarks)
		{
			if (idx++ != 0)
				res += "##"; // ## because reserved by imgui, so an input text cant have ##
			res += it.name + "##" + it.path;
		}

		return res;
	}

	void ImGuiFileDialog::DeserializeBookmarks(const std::string& vBookmarks)
	{
		if (!vBookmarks.empty())
		{
			m_Bookmarks.clear();
			auto arr = splitStringToVector(vBookmarks, '#', false);
			for (size_t i = 0; i < arr.size(); i += 2)
			{
				BookmarkStruct bookmark;
				bookmark.name = arr[i];
				if (i + 1 < arr.size()) // for avoid crash if arr size is impair due to user mistake after edition
				{
					// if bad format we jump this bookmark
					bookmark.path = arr[i + 1];
					m_Bookmarks.push_back(bookmark);
				}
			}
		}
	}
#endif
}

