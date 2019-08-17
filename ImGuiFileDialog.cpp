// From https://github.com/aiekick/ImGuiFileDialog
//
// Modified by AlastairGrowcott@yahoo.com to work on Posix systems.


#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <algorithm>

#include "imgui_file_dlg.h"
#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"


#define PATH_SEP '/'


static const std::string s_fs_root(1u, PATH_SEP);


inline void ReplaceString(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;

    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

inline std::vector<std::string> splitStringVector(const std::string& text, char delimiter)
{
    std::vector<std::string>  arr;
    size_t                    start = 0u;
    size_t                    end   = text.find(delimiter, start);

    while (end != std::string::npos)
    {
        std::string token("");

        if (0u == end)
        {
            token += delimiter;
        }
        else
        {
            token = text.substr(start, end - start);
        }

        if (!token.empty())
        {
            arr.push_back(token);
        }

        start = end + 1;
        end = text.find(delimiter, start);
    }
    arr.push_back(text.substr(start));
    return arr;
}

inline void AppendToBuffer(char* vBuffer, size_t vBufferLen, std::string vStr)
{
    std::string st = vStr;
    size_t len = vBufferLen - 1u;
    size_t slen = strlen(vBuffer);

    if (st != "" && st != "\n")
        ReplaceString(st, "\n", "");
    vBuffer[slen] = '\0';
    std::string str = std::string(vBuffer);
    if (str.size() > 0) str += "\n";
    str += vStr;
    if (len > str.size()) len = str.size();
#ifdef MINGW32
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
int ImGuiFileDialog::FilterIndex = 0;

ImGuiFileDialog::ImGuiFileDialog()
{

}

ImGuiFileDialog::~ImGuiFileDialog()
{

}

/* Alphabetical sorting */
static int alphaSort(const struct dirent **a, const struct dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}


void ImGuiFileDialog::ScanDir(std::string vPath)
{
    struct dirent **files               = NULL;
    int             i                   = 0;
    int             n                   = 0;

    m_FileList.clear();

    if (0u == m_CurrentPath_Decomposition.size())
    {
        SetCurrentDir(vPath);
    }

    if (0u != m_CurrentPath_Decomposition.size())
    {
        n = scandir(vPath.c_str(), &files, NULL, alphaSort);
        if (n > 0)
        {
            for (i = 0; i < n; i++)
            {
                struct dirent *ent = files[i];

                FileInfoStruct infos;

                infos.fileName = ent->d_name;
                if (("." != infos.fileName) && (".." != infos.fileName))
                {
                    switch (ent->d_type)
                    {
                        case DT_REG: infos.type = 'f'; break;
                        case DT_DIR: infos.type = 'd'; break;
                        case DT_LNK: infos.type = 'l'; break;
                    }

                    if (infos.type == 'f')
                    {
                        size_t lpt = infos.fileName.find_last_of(".");
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

//       std::sort(m_FileList.begin(), m_FileList.end(), stringComparator);
    }
}

void ImGuiFileDialog::SetCurrentDir(std::string vPath)
{
    DIR  *dir                 = opendir(vPath.c_str());
    char  real_path[PATH_MAX];

    if (NULL == dir)
    {
        vPath = ".";
        dir = opendir(vPath.c_str());
    }

    if (NULL != dir)
    {
        if (NULL != realpath(vPath.c_str(), real_path))
        {
            m_CurrentPath = real_path;
            m_CurrentPath_Decomposition = splitStringVector(m_CurrentPath, PATH_SEP);
        }

        closedir(dir);
    }
}

void ImGuiFileDialog::ComposeNewPath(std::vector<std::string>::iterator vIter)
{
    m_CurrentPath = "";

    while (true)
    {
        if (!m_CurrentPath.empty())
        {
            if (*vIter == s_fs_root)
            {
                m_CurrentPath = *vIter + m_CurrentPath;
            }
            else
            {
                m_CurrentPath = *vIter + PATH_SEP + m_CurrentPath;
            }
        }
        else
        {
            m_CurrentPath = *vIter;
        }

        if (vIter == m_CurrentPath_Decomposition.begin())
        {
            break;
        }

        vIter--;
    }
}

bool ImGuiFileDialog::FileDialog(const char* vName, const char* vFilters, std::string vPath, std::string vDefaultFileName)
{
    bool res = false;

    IsOk = false;

    ImGui::OpenPopup(vName);
    if (ImGui::BeginPopupModal(vName))
    {
        if (vPath.empty())
        {
            vPath = ".";
        }

        if (m_FileList.size() == 0)
        {
            if (vDefaultFileName.size() > 0)
            {
                ResetBuffer(FileNameBuffer);
                AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vDefaultFileName);
            }

            ScanDir(vPath);
        }

        // show current path
        bool pathClick = false;
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

        ImVec2 size = ImGui::GetContentRegionMax() - ImVec2(0.0f, 120.0f);

        ImGui::BeginChild("##FileDialog_FileList", size);

        for (std::vector<FileInfoStruct>::iterator it = m_FileList.begin(); it != m_FileList.end(); ++it)
        {
            FileInfoStruct infos = *it;

            bool show = true;

            std::string str;
            if (infos.type == 'd') str = "[Dir] " + infos.fileName;
            if (infos.type == 'l') str = "[Link] " + infos.fileName;
            if (infos.type == 'f') str = "[File] " + infos.fileName;
            if (infos.type == 'f' && m_CurrentFilterExt.size() > 0 && infos.ext != m_CurrentFilterExt)
            {
                show = false;
            }
            if (show == true)
            {
                if (ImGui::Selectable(str.c_str(), (infos.fileName == m_SelectedFileName)))
                {
                    if (infos.type == 'd')
                    {
                        if (s_fs_root == m_CurrentPath)
                        {
                            m_CurrentPath += infos.fileName;
                        }
                        else
                        {
                            m_CurrentPath += PATH_SEP + infos.fileName;
                        }

                        pathClick = true;
                    }
                    else
                    {
                        m_SelectedFileName = infos.fileName;
                        ResetBuffer(FileNameBuffer);
                        AppendToBuffer(FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, m_SelectedFileName);
                    }
                    break;
                }
            }
        }

        // changement de repertoire
        if (pathClick == true)
        {
            ScanDir(m_CurrentPath);
            m_CurrentPath_Decomposition = splitStringVector(m_CurrentPath, PATH_SEP);
            if (m_CurrentPath_Decomposition.size() == 2)
                if (m_CurrentPath_Decomposition[1] == "")
                    m_CurrentPath_Decomposition.erase(m_CurrentPath_Decomposition.end() - 1);
        }

        ImGui::EndChild();

        ImGui::Text("File Name : ");

        ImGui::SameLine();

        float width = ImGui::GetContentRegionAvailWidth();
        if (vFilters != 0) width -= 120.0f;
        ImGui::PushItemWidth(width);
        ImGui::InputText("##FileName", FileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
        ImGui::PopItemWidth();

        if (vFilters != 0)
        {
            ImGui::SameLine();

            ImGui::PushItemWidth(100.0f);
            bool comboClick = ImGui::Combo("##Filters", &FilterIndex, vFilters) || m_CurrentFilterExt.size() == 0;
            ImGui::PopItemWidth();
            if (comboClick == true)
            {
                int itemIdx = 0;
                const char* p = vFilters;
                while (*p)
                {
                    if (FilterIndex == itemIdx)
                    {
                        m_CurrentFilterExt = std::string(p);
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

        ImGui::SameLine();

        if (ImGui::Button("Ok"))
        {
            if ('\0' != FileNameBuffer[0])
            {



                IsOk = true;
                res = true;
            }
        }

        ImGui::EndPopup();
    }

    if (res == true)
    {
        m_FileList.clear();
    }

    return res;
}

void ImGuiFileDialog::Reset()
{
    m_FileList.clear();
    m_SelectedFileName.clear();
    m_CurrentPath.clear();
    m_CurrentPath_Decomposition.clear();
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
    size_t      pos;

    pos = result.size() - m_CurrentFilterExt.size();
    if (m_CurrentFilterExt != result.substr(pos)) {
        result += m_CurrentFilterExt;
    }
    
    return result;
}

std::string ImGuiFileDialog::GetCurrentFilter()
{
    return m_CurrentFilterExt;
}


