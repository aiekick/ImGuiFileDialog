#include "Test_FileStyle.h"

#include <cassert>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

#pragma region Helper

/*class FileStyleTestHelper {
private:
    IGFD::FileDialogInternal fd;
    std::set<std::string> arr;

private:
    void compute_name_array() {
        for (const auto& file_ptr : fd.puFileStyle.prFileList) {
            if (file_ptr) {
                arr.emplace(file_ptr->fileNameExt);
            }
        }
    }

public:
    void addFilter(const std::string& vFilter) {
        fd.filterManager.ParseFilters(vFilter.c_str());
        fd.filterManager.SetSelectedFilterWithExt(vFilter);
    }

    void addFile(const std::string& vFile) {
        fd.puFileStyle.AddFile(fd, ".", vFile, IGFD::FileType(IGFD::FileType::ContentType::File, false));
    };

    void addFiles(const std::vector<std::string>& vFiles) {
        for (const auto& file : vFiles) {
            addFile(file);
        }
    };


    bool isFileExist(const std::string& vFile) {
        if (arr.empty()) {
            compute_name_array();
        }
        return (arr.find(vFile) != arr.end());
    }
};  */

#pragma endregion

#pragma region By Extention

bool Test_IGFD_FileStyle_ByExtention_0() {
    /* FileStyleTestHelper mgr;
    mgr.addFilter(".*");
    mgr.addFiles({"toto.a", "toto.b","titi.c"});

    if (!mgr.isFileExist("toto.a")) return false;
    if (!mgr.isFileExist("toto.b")) return false;
    if (!mgr.isFileExist("titi.c")) return false;*/

    return true;
}

#pragma endregion

#pragma region Entry Point

#define IfTestExist(v) \
    if (vTest == std::string(#v)) return v()

bool Test_FileStyle(const std::string& vTest) {
    IfTestExist(Test_IGFD_FileStyle_ByExtention_0);
    //IfTestExist(Test_IGFD_FileStyle_ByExtention_1);
    //IfTestExist(Test_IGFD_FileStyle_ByExtention_2);
    //IfTestExist(Test_IGFD_FileStyle_ByExtention_3);
    //IfTestExist(Test_IGFD_FileStyle_ByExtention_4);
    //IfTestExist(Test_IGFD_FileStyle_ByExtention_5);

    assert(0);

    return false;
}

#pragma endregion
