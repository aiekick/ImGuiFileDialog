// From https://github.com/aiekick/ImGuiFileDialog
//
// Modified by AlastairGrowcott@yahoo.com to work on Posix systems.


#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include "imgui_file_dlg.h"
#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"


#define PATH_SEP                     '/'
#define MAX_FILE_DIALOG_NAME_BUFFER  (1024u)


struct FileInfoStruct
{
	char type;
	std::string name;
	std::string ext;
};


static const std::string            s_fs_root(1u, PATH_SEP);
static bool                         s_accepted                                 = false;
static std::vector<FileInfoStruct>  s_file_list;
static std::string                  s_selected_file_name;
static std::string                  s_current_path;
static std::vector<std::string>     s_path_decomposition;
static std::string                  s_current_filter;
static char                         s_name_buffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
static int                          s_filter_index                             = 0;


static int alpha_sort(const struct dirent **a, const struct dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}


static void replace_string(std::string& str, const std::string& old_str, const std::string& new_str)
{
    size_t pos = 0;

    while ((pos = str.find(old_str, pos)) != std::string::npos) {
        str.replace(pos, old_str.length(), new_str);
        pos += new_str.length();
    }
}


static std::vector<std::string> split_string(const std::string& text, char delimiter)
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

static void append_to_buffer(char *buffer, size_t len, std::string affix)
{
    std::string  str = std::string(buffer);

    if (!str.empty())
    {
        str += "\n";
    }

    if ((affix != "") && (affix != "\n"))
    {
        replace_string(affix, "\n", "");
        str += affix;
    }

    
    if (len > str.size())
    {
        len = str.size();
    } else {
        // Allow room for null terminator.
        len--;
    }

#ifdef MINGW32
    strncpy_s(buffer, len, str.c_str(), len);
#else
    strncpy(buffer, str.c_str(), len);
#endif

    buffer[len] = '\0';
}


static void set_current_dir(std::string vPath)
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
            s_current_path = real_path;
            s_path_decomposition = split_string(s_current_path, PATH_SEP);
        }

        closedir(dir);
    }
}

static void compose_new_path(std::vector<std::string>::iterator vIter)
{
    s_current_path = "";

    while (true)
    {
        if (!s_current_path.empty())
        {
            if (*vIter == s_fs_root)
            {
                s_current_path = *vIter + s_current_path;
            }
            else
            {
                s_current_path = *vIter + PATH_SEP + s_current_path;
            }
        }
        else
        {
            s_current_path = *vIter;
        }

        if (vIter == s_path_decomposition.begin())
        {
            break;
        }

        vIter--;
    }
}


static void scan_dir(std::string vPath)
{
    struct dirent **files               = NULL;
    int             i                   = 0;
    int             n                   = 0;

    s_file_list.clear();

    if (0u == s_path_decomposition.size())
    {
        set_current_dir(vPath);
    }

    if (0u != s_path_decomposition.size())
    {
        n = scandir(vPath.c_str(), &files, NULL, alpha_sort);
        if (n > 0)
        {
            for (i = 0; i < n; i++)
            {
                struct dirent *ent = files[i];

                FileInfoStruct infos;

                infos.name = ent->d_name;
                if (("." != infos.name) && (".." != infos.name))
                {
                    switch (ent->d_type)
                    {
                        case DT_REG: infos.type = 'f'; break;
                        case DT_DIR: infos.type = 'd'; break;
                        case DT_LNK: infos.type = 'l'; break;
                    }

                    if (infos.type == 'f')
                    {
                        size_t lpt = infos.name.find_last_of(".");
                        if (lpt != std::string::npos)
                        {
                            infos.ext = infos.name.substr(lpt);
                        }
                    }

                    s_file_list.push_back(infos);
                }
            }

            for (i = 0; i < n; i++)
            {
                free(files[i]);
            }
            free(files);
        }
    }
}


bool ImGui::FileDialogAccepted()
{
    return s_accepted;
}

bool ImGui::FileDialog(const char* vName, const char* vFilters, std::string vPath, std::string vDefaultFileName)
{
    bool result = false;

    s_accepted = false;

    ImGui::OpenPopup(vName);
    if (ImGui::BeginPopupModal(vName))
    {
        if (240.0 > ImGui::GetWindowHeight())
        {
            ImGui::SetWindowSize(ImVec2(0.0, 240.0));
        }

        if (vPath.empty())
        {
            vPath = ".";
        }

        if (s_path_decomposition.empty())
        {
            if (vDefaultFileName.size() > 0)
            {
                s_name_buffer[0] = '\0';
                append_to_buffer(s_name_buffer, MAX_FILE_DIALOG_NAME_BUFFER, vDefaultFileName);
            }

            s_file_list.clear();
            scan_dir(vPath);
        }

        // show current path
        bool pathClick = false;
        for (std::vector<std::string>::iterator itPathDecomp = s_path_decomposition.begin();
                itPathDecomp != s_path_decomposition.end(); ++itPathDecomp)
        {
            if (itPathDecomp != s_path_decomposition.begin())
                ImGui::SameLine(); 
            if (ImGui::Button((*itPathDecomp).c_str()))
            {
                compose_new_path(itPathDecomp);
                pathClick = true;
                break;
            }
        }

        ImGui::BeginChild("##FileDialog_FileList", ImVec2(0.0f, -44.0f));

        for (std::vector<FileInfoStruct>::iterator it = s_file_list.begin(); it != s_file_list.end(); ++it)
        {
            FileInfoStruct infos = *it;

            bool show = true;

            std::string str;
            if (infos.type == 'd') str = "[Dir] " + infos.name;
            if (infos.type == 'l') str = "[Link] " + infos.name;
            if (infos.type == 'f') str = "[File] " + infos.name;
            if (infos.type == 'f' && s_current_filter.size() > 0 && infos.ext != s_current_filter)
            {
                show = false;
            }
            if (show == true)
            {
                if (ImGui::Selectable(str.c_str(), (infos.name == s_selected_file_name)))
                {
                    if (infos.type == 'd')
                    {
                        if (s_fs_root == s_current_path)
                        {
                            s_current_path += infos.name;
                        }
                        else
                        {
                            s_current_path += PATH_SEP + infos.name;
                        }

                        pathClick = true;
                    }
                    else
                    {
                        s_selected_file_name = infos.name;
                        s_name_buffer[0] = '\0';
                        append_to_buffer(s_name_buffer, MAX_FILE_DIALOG_NAME_BUFFER, s_selected_file_name);
                    }
                    break;
                }
            }
        }

        // changement de repertoire
        if (pathClick == true)
        {
            scan_dir(s_current_path);
            s_path_decomposition = split_string(s_current_path, PATH_SEP);
            if (s_path_decomposition.size() == 2)
                if (s_path_decomposition[1] == "")
                    s_path_decomposition.erase(s_path_decomposition.end() - 1);
        }

        ImGui::EndChild();

        ImGui::Text("File Name : ");

        ImGui::SameLine();

        float width = ImGui::GetContentRegionAvailWidth();
        if (vFilters != 0) width -= 120.0f;
        ImGui::PushItemWidth(width);
        ImGui::InputText("##FileName", s_name_buffer, MAX_FILE_DIALOG_NAME_BUFFER);
        ImGui::PopItemWidth();

        if (vFilters != 0)
        {
            ImGui::SameLine();

            ImGui::PushItemWidth(100.0f);
            bool comboClick = ImGui::Combo("##Filters", &s_filter_index, vFilters) || s_current_filter.size() == 0;
            ImGui::PopItemWidth();
            if (comboClick == true)
            {
                int itemIdx = 0;
                const char* p = vFilters;
                while (*p)
                {
                    if (s_filter_index == itemIdx)
                    {
                        s_current_filter = std::string(p);
                        break;
                    }
                    p += strlen(p) + 1;
                    itemIdx++;
                }
            }
        }

        if (ImGui::Button("Cancel"))
        {
            s_accepted = false;
            result = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Ok"))
        {
            if ('\0' != s_name_buffer[0])
            {
                s_accepted = true;
                result = true;
            }
        }

        ImGui::EndPopup();
    }

    if (result == true)
    {
        s_path_decomposition.clear();
        s_file_list.clear();
    }

    return result;
}


std::string ImGui::FileDialogFullPath()
{
    std::string  result = s_current_path;

    if (s_fs_root != result)
    {
        result += PATH_SEP;
    }

    result += FileDialogFilename();

    return result;
}


std::string ImGui::FileDialogPath()
{
    return s_current_path;
}


std::string ImGui::FileDialogFilename()
{
    std::string result = s_name_buffer;
    size_t      pos;

    pos = result.size() - s_current_filter.size();
    if (s_current_filter != result.substr(pos)) {
        result += s_current_filter;
    }
    
    return result;
}


std::string ImGui::FileDialogFilter()
{
    return s_current_filter;
}


void ImGui::FileDialogReset()
{
    s_file_list.clear();
    s_selected_file_name.clear();
    s_name_buffer[0] = '\0';
    s_current_path.clear();
    s_path_decomposition.clear();
}


