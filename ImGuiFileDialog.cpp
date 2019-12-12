// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ImGuiFileDialog.h"
#include "imgui.h"

#ifdef WIN32
#include <include/dirent.h>
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

static std::string s_fs_root(1u, PATH_SEP);

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
	if (text.size() > 0)
	{
		std::string::size_type start = 0;
		std::string::size_type end = text.find(delimiter, start);
		while (end != std::string::npos)
		{
			std::string token = text.substr(start, end - start);
			if (token.size() > 0 || (token.size() == 0 && pushEmpty))
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

	DWORD countChars = GetLogicalDriveStrings(mydrives, lpBuffer);

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

	if (name.size() > 0)
	{
		DIR *pDir = 0;
		pDir = opendir(name.c_str());
		if (pDir != NULL)
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

	if (name.size() > 0)
	{
		if (!IsDirectoryExist(name))
		{
			res = true;

#ifdef WIN32
			CreateDirectory(name.c_str(), NULL);
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

	if (vPathFileName.size() > 0)
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

inline void AppendToBuffer(char* vBuffer, size_t vBufferLen, std::string vStr)
{
    std::string st = vStr;
    size_t len = vBufferLen - 1u;
    size_t slen = strlen(vBuffer);

    if (st != "" && st != "\n")
    {
        replaceString(st, "\n", "");
        replaceString(st, "\r", "");
    }
    vBuffer[slen] = '\0';
    std::string str = std::string(vBuffer);
    if (str.size() > 0) str += "\n";
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
	dlg_optionsPane = 0;
	dlg_optionsPaneWidth = 250;
	dlg_filters = "";
}

ImGuiFileDialog::~ImGuiFileDialog()
{

}

/* Alphabetical sorting */
/*#ifdef WIN32
static int alphaSort(const void *a, const void *b)
{
	return strcoll(((dirent*)a)->d_name, ((dirent*)b)->d_name);
}
#elif defined(LINUX) or defined(APPLE)*/
static int alphaSort(const struct dirent **a, const struct dirent **b)
{
	return strcoll((*a)->d_name, (*b)->d_name);
}
//#endif

static bool stringComparator(const FileInfoStruct& a, const FileInfoStruct& b)
{
	bool res;
	if (a.type != b.type) res = (a.type < b.type);
	else res = (a.fileName < b.fileName);
	return res;
}

void ImGuiFileDialog::ScanDir(const std::string& vPath)
{
    struct dirent **files               = NULL;
    int             i                   = 0;
    int             n                   = 0;
	std::string		path				= vPath;

#if defined(LINUX) or defined(APPLE)
    if (path.size()>0)
    {
        if (path[0] != PATH_SEP)
        {
            //path = PATH_SEP + path;
        }
    }
#endif

    if (0u == m_CurrentPath_Decomposition.size())
    {
        SetCurrentDir(path);
    }

    if (0u != m_CurrentPath_Decomposition.size())
    {
#ifdef WIN32
		if (path == s_fs_root)
		{
			path += PATH_SEP;
		}
#endif
        n = scandir(path.c_str(), &files, NULL, alphaSort);
        if (n > 0)
        {
			m_FileList.clear();

			for (i = 0; i < n; i++)
            {
                struct dirent *ent = files[i];

                FileInfoStruct infos;

                infos.fileName = ent->d_name;
                if (("." != infos.fileName)/* && (".." != infos.fileName)*/)
                {
                    switch (ent->d_type)
                    {
                        case DT_REG: infos.type = 'f'; break;
                        case DT_DIR: infos.type = 'd'; break;
                        case DT_LNK: infos.type = 'l'; break;
                    }

                    if (infos.type == 'f')
                    {
                        size_t lpt = infos.fileName.find_last_of('.');
                        if (lpt != std::string::npos)
                        {
                            infos.ext = infos.fileName.substr(lpt);
                        }
                    }

                    m_FileList.push_back(infos);
                }
            }

            for (i = 0; i < n; i++)
            {
                free(files[i]);
            }
            free(files);
        }

		std::sort(m_FileList.begin(), m_FileList.end(), stringComparator);
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

    if (NULL == dir)
    {
		path = ".";
        dir = opendir(path.c_str());
    }

    if (NULL != dir)
    {
#ifdef WIN32
		size_t numchar = GetFullPathName(path.c_str(), PATH_MAX-1, real_path, 0);
#elif defined(LINUX) or defined(APPLE)
		char *numchar = realpath(path.c_str(), real_path);
#endif
		if (numchar != 0)
        {
			m_CurrentPath = real_path;
			if (m_CurrentPath[m_CurrentPath.size()-1] == PATH_SEP)
			{
				m_CurrentPath = m_CurrentPath.substr(0, m_CurrentPath.size() - 1);
			}
			m_CurrentPath_Decomposition = splitStringToVector(m_CurrentPath, PATH_SEP, false);
#if defined(LINUX) or defined(APPLE)
			m_CurrentPath_Decomposition.insert(m_CurrentPath_Decomposition.begin(), std::string(1u, PATH_SEP));
#endif
			if (m_CurrentPath_Decomposition.size()>0)
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

	if (vPath.size())
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
#if defined(LINUX) or defined(APPLE)
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
	if (res.size() > 0)
	{
		m_CurrentPath = "";
		m_CurrentPath_Decomposition.clear();
		m_FileList.clear();
		for (auto it = res.begin(); it != res.end(); ++it)
		{
			FileInfoStruct infos;
			infos.fileName = *it;
			infos.type = 'd';

			if (infos.fileName.size() > 0)
			{
				m_FileList.push_back(infos);
			}
		}
		m_ShowDrives = true;
	}
}

void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
	const std::string& vPath, const std::string& vDefaultFileName,
	std::function<void(std::string, bool*)> vOptionsPane, size_t vOptionsPaneWidth, const std::string& vUserString)
{
	if (m_ShowDialog) // si deja ouvert on ne fou pas la merde en voulant en ecrire une autre
		return;

	dlg_key = vKey;
	dlg_name = std::string(vName);
	dlg_filters = vFilters;
	dlg_path = vPath;
	dlg_defaultFileName = vDefaultFileName;
	dlg_optionsPane = vOptionsPane;
	dlg_userString = vUserString;
	dlg_optionsPaneWidth = vOptionsPaneWidth;

	dlg_defaultExt = "";

	m_ShowDialog = true;
}

void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
	const std::string& vFilePathName,
	std::function<void(std::string, bool*)> vOptionsPane, size_t vOptionsPaneWidth, const std::string& vUserString)
{
	if (m_ShowDialog) // si deja ouvert on ne fou pas la merde en voulant en ecrire une autre
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

	dlg_optionsPane = vOptionsPane;
	dlg_userString = vUserString;
	dlg_optionsPaneWidth = vOptionsPaneWidth;

	m_ShowDialog = true;
}

void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
	const std::string& vFilePathName, const std::string& vUserString)
{
	if (m_ShowDialog) // si deja ouvert on ne fou pas la merde en voulant en ecrire une autre
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

	dlg_optionsPane = 0;
	dlg_userString = vUserString;
	dlg_optionsPaneWidth = 0;

	m_ShowDialog = true;
}

void ImGuiFileDialog::OpenDialog(const std::string& vKey, const char* vName, const char* vFilters,
	const std::string& vPath, const std::string& vDefaultFileName, const std::string& vUserString)
{
	if (m_ShowDialog) // si deja ouvert on ne fou pas la merde en voulant en ecrire une autre
		return;

	dlg_key = vKey;
	dlg_name = std::string(vName);
	dlg_filters = vFilters;
	dlg_path = vPath;
	dlg_defaultFileName = vDefaultFileName;
	dlg_optionsPane = 0;
	dlg_userString = vUserString;
	dlg_optionsPaneWidth = 0;

	dlg_defaultExt = "";

	m_ShowDialog = true;
}

void ImGuiFileDialog::CloseDialog(const std::string& vKey)
{
	if (dlg_key == vKey)
	{
		dlg_key.clear();
		m_ShowDialog = false;
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

bool ImGuiFileDialog::FileDialog(const std::string& vKey)
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

		if (ImGui::Begin(name.c_str(), (bool*)0, ImGuiWindowFlags_Modal |
			ImGuiWindowFlags_NoCollapse /*| ImGuiWindowFlags_NoDocking*/))
		{

			m_Name = name;

			m_AnyWindowsHovered |= ImGui::IsWindowHovered();

			if (dlg_path.size() == 0) dlg_path = ".";

			if (m_FileList.size() == 0 && !m_ShowDrives)
			{
				replaceString(dlg_defaultFileName, dlg_path, ""); // local path

				if (dlg_defaultFileName.size() > 0)
				{
					ResetBuffer(FileNameBuffer);
					AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, dlg_defaultFileName);
					m_SelectedFileName = dlg_defaultFileName;

					if (dlg_defaultExt.size() > 0)
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
						for (auto it = arr.begin(); it != arr.end(); ++it)
						{
							if (m_SelectedExt == *it)
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

			if (ImGui::Button("+"))
			{
				if (!m_CreateDirectoryMode)
				{
					m_CreateDirectoryMode = true;
					ResetBuffer(DirectoryNameBuffer);
				}
			}

			if (m_CreateDirectoryMode)
			{
				ImGui::SameLine();

				ImGui::PushItemWidth(100.0f);
				ImGui::InputText("##DirectoryFileName", DirectoryNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
				ImGui::PopItemWidth();

				ImGui::SameLine();

				if (ImGui::Button("OK"))
				{
					std::string newDir = std::string(DirectoryNameBuffer);
					if (CreateDir(newDir))
					{
						SetPath(m_CurrentPath + PATH_SEP + newDir);
					}

					m_CreateDirectoryMode = false;
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel"))
				{
					m_CreateDirectoryMode = false;
				}
			}

			ImGui::SameLine();

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			ImGui::SameLine();

			if (ImGui::Button("R"))
			{
				SetPath(".");
			}

			bool drivesClick = false;

#ifdef WIN32
			ImGui::SameLine();

			if (ImGui::Button("Drives"))
			{
				drivesClick = true;
			}
#endif

			ImGui::SameLine();

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			// show current path
			bool pathClick = false;
			if (m_CurrentPath_Decomposition.size() > 0)
			{
				ImGui::SameLine();
				for (std::vector<std::string>::iterator itPathDecomp = m_CurrentPath_Decomposition.begin();
					itPathDecomp != m_CurrentPath_Decomposition.end(); ++itPathDecomp)
				{
					if (itPathDecomp != m_CurrentPath_Decomposition.begin())
						ImGui::SameLine();
					if (ImGui::Button((*itPathDecomp).c_str()))
					{
						ComposeNewPath(itPathDecomp);
						pathClick = true;
						break;
					}
				}
			}

			ImVec2 size = ImGui::GetContentRegionMax() - ImVec2((float)dlg_optionsPaneWidth, 120.0f);

			// search field
			if (ImGui::Button("R##ImGuiFileDialogSearchFiled"))
			{
				ResetBuffer(SearchBuffer);
				searchTag.clear();
			}
			ImGui::SameLine();
			ImGui::Text("Search : ");
			ImGui::SameLine();
			if (ImGui::InputText("##ImGuiFileDialogSearchFiled", SearchBuffer, MAX_FILE_DIALOG_NAME_BUFFER))
			{
				searchTag = SearchBuffer;
			}

			ImGui::BeginChild("##FileDialog_FileList", size);

			for (std::vector<FileInfoStruct>::iterator it = m_FileList.begin(); it != m_FileList.end(); ++it)
			{
				const FileInfoStruct& infos = *it;

				bool show = true;

				std::string str;
				if (infos.type == 'd') str = "[Dir] " + infos.fileName;
				if (infos.type == 'l') str = "[Link] " + infos.fileName;
				if (infos.type == 'f') str = "[File] " + infos.fileName;
				if (infos.type == 'f' && m_SelectedExt.size() > 0 && (infos.ext != m_SelectedExt && m_SelectedExt != ".*"))
				{
					show = false;
				}
				if (searchTag.size() > 0 && infos.fileName.find(searchTag) == std::string::npos)
				{
					show = false;
				}
				if (show == true)
				{
				    ImVec4 c;
				    bool showColor = GetFilterColor(infos.ext, &c);
				    if (showColor)
				        ImGui::PushStyleColor(ImGuiCol_Text, c);

					if (ImGui::Selectable(str.c_str(), (infos.fileName == m_SelectedFileName)))
					{
						if (infos.type == 'd')
						{
							if ((*it).fileName == "..")
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
									newPath = infos.fileName + PATH_SEP;
								}
								else
								{
#ifdef LINUX
									if (s_fs_root == m_CurrentPath)
									{
										newPath = m_CurrentPath + infos.fileName;
									}
									else
									{
#endif
										newPath = m_CurrentPath + PATH_SEP + infos.fileName;
#ifdef LINUX
									}
#endif
								}

								if (IsDirectoryExist(newPath))
								{
									if (m_ShowDrives)
									{
										m_CurrentPath = infos.fileName;
										s_fs_root = m_CurrentPath;
									}
									else
									{
										m_CurrentPath = newPath;
									}
									pathClick = true;
								}
							}
						}
						else
						{
							m_SelectedFileName = infos.fileName;
							ResetBuffer(FileNameBuffer);
							AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, m_SelectedFileName);
						}
						 if (showColor)
                        ImGui::PopStyleColor();
						break;
					}

                    if (showColor)
                        ImGui::PopStyleColor();
                }
			}

			// changement de repertoire
			if (pathClick == true)
			{
				SetPath(m_CurrentPath);
			}

			if (drivesClick == true)
			{
				GetDrives();
			}

			ImGui::EndChild();

			bool _CanWeContinue = true;

			if (dlg_optionsPane)
			{
				ImGui::SameLine();

				size.x = (float)dlg_optionsPaneWidth;

				ImGui::BeginChild("##FileTypes", size);

				dlg_optionsPane(m_SelectedExt, &_CanWeContinue);

				ImGui::EndChild();
			}

			ImGui::Text("File Name : ");

			ImGui::SameLine();

			float width = ImGui::GetContentRegionAvail().x;
			if (dlg_filters) width -= 120.0f;
			ImGui::PushItemWidth(width);
			ImGui::InputText("##FileName", FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
			ImGui::PopItemWidth();

			if (dlg_filters)
			{
				ImGui::SameLine();

				ImGui::PushItemWidth(100.0f);
				bool comboClick = ImGui::Combo("##Filters", &FilterIndex, dlg_filters) || m_SelectedExt.size() == 0;
				ImGui::PopItemWidth();
				if (comboClick == true)
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

			if (ImGui::Button("Cancel"))
			{
				IsOk = false;
				res = true;
			}

			if (_CanWeContinue)
			{
				ImGui::SameLine();

				if (ImGui::Button("Ok"))
				{
					if ('\0' != FileNameBuffer[0])
					{
						IsOk = true;
						res = true;
					}
				}
			}
		}

		ImGui::End();

		if (res == true)
		{
			m_FileList.clear();
		}

		return res;
	}

	return false;
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

std::string ImGuiFileDialog::GetUserString()
{
	return dlg_userString;
}

void ImGuiFileDialog::SetFilterColor(std::string vFilter, ImVec4 vColor)
{
    m_FilterColor[vFilter] = vColor;
}

bool ImGuiFileDialog::GetFilterColor(std::string vFilter, ImVec4 *vColor)
{
    if (vColor)
    {
        if (m_FilterColor.find(vFilter) != m_FilterColor.end()) // found
        {
            *vColor = m_FilterColor[vFilter];
            return true;
        }
    }
    return false;;
}

void ImGuiFileDialog::ClearFilterColor()
{
    m_FilterColor.clear();
}
