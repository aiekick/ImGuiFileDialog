// From https://github.com/aiekick/ImGuiFileDialog
//
// Modified by AlastairGrowcott@yahoo.com to work on Posix systems.


#ifndef __IMGUI_FILE_DIALOG_H_
#define __IMGUI_FILE_DIALOG_H_


#include <cstdbool>
#include <vector>
#include <string>


namespace ImGui
{

    // The filters are a null-terminator separated and terminated list of file extensions. Each filter should
    // start with a period. E.g.:
    //     ".txt\0.c\0.cpp\0.h\0"
    bool FileDialog(const char  *vName,
                    const char  *vFilters = 0,
                    std::string  vPath = ".",
                    std::string  vDefaultFileName = "");

    bool FileDialogAccepted();

    std::string FileDialogFullPath();
    std::string FileDialogPath();
    std::string FileDialogFilename();
    std::string FileDialogFilter();

    void FileDialogReset();

};


#endif // __IMGUI_FILE_DIALOG_H_

