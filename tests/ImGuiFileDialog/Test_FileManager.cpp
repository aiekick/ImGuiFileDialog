#include "Test_FileManager.h"

#include <cassert>

#include <ImGuiFileDialog/ImGuiFileDialog.cpp> // for let IGFD::FileManager find the FS on destroy

#include <imgui_internal.h>

#include <filesystem>

// specific
#ifdef WIN32
#include <direct.h>  // _chdir
#define GetCurrentDir _getcwd
#else
#include <unistd.h>  // chdir
#define GetCurrentDir getcwd
#endif

#pragma region Helper

namespace IGFD {

class TestFileManager {
private:
    FileDialogInternal fd;
    std::set<std::string> arr;

private:
    void m_compute_name_array() {
        for (const auto& file_ptr : fd.fileManager.m_FileList) {
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
        fd.fileManager.m_AddFile(fd, ".", vFile, FileType(FileType::ContentType::File, false));
    };

    void addFiles(const std::vector<std::string>& vFiles) {
        for (const auto& file : vFiles) {
            addFile(file);
        }
    };

    void createDirectoryOnDisk(const std::string& vDirectory) {
        fd.fileManager.m_FileSystemPtr->CreateDirectoryIfNotExist(vDirectory);
    }
    
    void createFileOnDisk(const std::string& vFile) {
        std::ofstream fileWriter(vFile, std::ios::out);
        if (!fileWriter.bad()) {
            fileWriter << "for testing";
            fileWriter.close();
        }
    }
    
    bool isFileExist(const std::string& vFile) {
        if (arr.empty()) {
            m_compute_name_array();
        }
        return (arr.find(vFile) != arr.end());
    }

    void useFlags(ImGuiFileDialogFlags vFlags) {
        fd.getDialogConfigRef().flags = vFlags;
    }
    
    FilterInfos getSelectedFilter() const {
        return fd.filterManager.GetSelectedFilter();
    }

    FileDialogInternal& getFileDialogInternal() {
        return fd;
    }

    void setInput(const std::string& vInput) {
        if (!vInput.empty()) {
            ImFormatString(fd.fileManager.fileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vInput.c_str());
        } else {
            fd.fileManager.fileNameBuffer[0] = '\0';
        }        
    }

    void setCurrentPath(const std::string& vPath) {
        fd.fileManager.m_CurrentPath = vPath;
    }

    std::string getCurrentDir() {
        char buffer[1024 + 1] = "\0";
        GetCurrentDir(buffer, 1024);
        return std::string(buffer);
    }
};
}  // namespace IGFD

#pragma endregion

#pragma region Filtering for .* patterns

bool Test_IGFD_FileManager_Filtering_asterisk_0() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".*");
    mgr.addFiles({"toto.a", "toto.b","titi.c"});

    if (!mgr.isFileExist("toto.a")) return false;
    if (!mgr.isFileExist("toto.b")) return false;
    if (!mgr.isFileExist("titi.c")) return false;

    return true;
}

bool Test_IGFD_FileManager_Filtering_asterisk_1() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".*.*");
    mgr.addFiles({"toto.a.b", "toto.b.c", "titi.c"});

    if (!mgr.isFileExist("toto.a.b")) return false;
    if (!mgr.isFileExist("toto.b.c")) return false;
    if (mgr.isFileExist("titi.c")) return false;

    return true;
}

bool Test_IGFD_FileManager_Filtering_asterisk_2() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".a.*");
    mgr.addFiles({"toto.a.b", "toto.b.c", "titi.a.c"});

    if (!mgr.isFileExist("toto.a.b")) return false;
    if (mgr.isFileExist("toto.b.c")) return false;
    if (!mgr.isFileExist("titi.a.c")) return false;

    return true;
}

bool Test_IGFD_FileManager_Filtering_asterisk_3() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".*.b");
    mgr.addFiles({"toto.a.b", "toto.c.b", "titi.a.bb"});

    if (!mgr.isFileExist("toto.a.b")) return false;
    if (!mgr.isFileExist("toto.c.b")) return false;
    if (mgr.isFileExist("titi.a.bb")) return false;

    return true;
}

#pragma endregion

#pragma region Filtering Divers

bool Test_IGFD_FileManager_Filtering_divers_0() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".a.b");
    mgr.addFiles({"toto.a.b", "toto.b.c", "titi.a.c"});

    if (!mgr.isFileExist("toto.a.b")) return false;
    if (mgr.isFileExist("toto.b.c")) return false;
    if (mgr.isFileExist("titi.a.c")) return false;

    return true;
}

bool Test_IGFD_FileManager_Filtering_divers_1() {
    IGFD::TestFileManager mgr;
    mgr.addFilter("{.filters, .a.b}");
    mgr.addFiles({"toto.a.filters", "titi.a.b", "toto.c.a.b", "tata.t.o", "toto.filters"});

    if (!mgr.isFileExist("toto.filters")) return false;
    if (!mgr.isFileExist("titi.a.b")) return false;
    if (!mgr.isFileExist("toto.c.a.b")) return false;
    if (mgr.isFileExist("tata.t.o")) return false;
    if (!mgr.isFileExist("toto.a.filters")) return false;

    return true;
}

bool Test_IGFD_FileManager_Filtering_divers_2() {
    IGFD::TestFileManager mgr;
    mgr.addFilter("Shader files{.glsl,.comp,.vert,.frag}");
    mgr.addFiles({"toto.comp", "titi.vert", "toto.frag", "tata.glsl"});

    if (!mgr.isFileExist("toto.comp")) return false;
    if (!mgr.isFileExist("titi.vert")) return false;
    if (!mgr.isFileExist("toto.frag")) return false;
    if (!mgr.isFileExist("tata.glsl")) return false;
    if (mgr.getSelectedFilter().getFirstFilter() != ".glsl") return false;

    return true;
}

#pragma endregion

#pragma region Filtering Sensitive/Insensitive case

// issue #140
bool Test_IGFD_FileManager_Filtering_sensitive_case_0() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".cpp");
    mgr.addFiles({"toto.cpp", "titi.CPP"});

    if (!mgr.isFileExist("toto.cpp")) return false;
    if (mgr.isFileExist("titi.CPP")) return false;

    return true;
}

// issue #140
bool Test_IGFD_FileManager_Filtering_sensitive_case_1() {
    IGFD::TestFileManager mgr;
    mgr.addFilter(".CPP");
    mgr.addFiles({"toto.cpp", "titi.CPP"});

    if (mgr.isFileExist("toto.cpp")) return false;
    if (!mgr.isFileExist("titi.CPP")) return false;

    return true;
}

// issue #140
bool Test_IGFD_FileManager_Filtering_insensitive_case_0() {
    IGFD::TestFileManager mgr;
    mgr.useFlags(ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering);
    mgr.addFilter(".cpp");
    mgr.addFiles({"toto.cpp", "titi.CPP"});

    if (!mgr.isFileExist("toto.cpp")) return false;
    if (!mgr.isFileExist("titi.CPP")) return false;

    return true;
}

// issue #140
bool Test_IGFD_FileManager_Filtering_insensitive_case_1() {
    IGFD::TestFileManager mgr;
    mgr.useFlags(ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering);
    mgr.addFilter(".CPP");
    mgr.addFiles({"toto.cpp", "titi.CPP"});

    if (!mgr.isFileExist("toto.cpp")) return false;
    if (!mgr.isFileExist("titi.CPP")) return false;

    return true;
}

#pragma endregion

#pragma region FileInfos SearchForExts

bool Test_IGFD_FileManager_FileInfos_SearchForExts_0() {
    auto infos_ptr = IGFD::FileInfos::create();
    infos_ptr->fileType.SetContent(IGFD::FileType::ContentType::File);
    const std::string& filters = ".png,.gif,.jpg";
    infos_ptr->fileNameExt = "toto_0.png";
    if (!infos_ptr->FinalizeFileTypeParsing(1)) return false;
    if (!infos_ptr->SearchForExts(filters, false)) return false;
    infos_ptr->fileNameExt = "toto_0.0.png";
    if (!infos_ptr->FinalizeFileTypeParsing(1)) return false;
    if (!infos_ptr->SearchForExts(filters, false)) return false;

    return true;
}

#pragma endregion

#pragma region GetResultingFilePathName

// ensure #144
bool Test_IGFD_FileManager_GetResultingFilePathName_issue_144() {
    IGFD::TestFileManager mgr;
    auto& fd = mgr.getFileDialogInternal();
    mgr.setInput("toto.cpp");
    if (fd.fileManager.GetResultingFileName(fd, IGFD_ResultMode_AddIfNoFileExt) != "toto.cpp") return false;
    mgr.createDirectoryOnDisk("TITI");
    const std::string& existing_path_file = "TITI" + IGFD::Utils::GetPathSeparator() + "toto.cpp ";
    mgr.createFileOnDisk(existing_path_file);
    mgr.setInput(existing_path_file);
    if (fd.fileManager.GetResultingFilePathName(fd, IGFD_ResultMode_AddIfNoFileExt) != existing_path_file) return false;
    return true;
}

// ensure #184
bool Test_IGFD_FileManager_GetResultingFilePathName_issue_184() {
    IGFD::TestFileManager mgr;
    auto& fd = mgr.getFileDialogInternal();
    mgr.setInput("toto.cpp");
    if (fd.fileManager.GetResultingFileName(fd, IGFD_ResultMode_AddIfNoFileExt) != "toto.cpp") return false;
    mgr.createFileOnDisk("toto.cpp");  // we create a local file toto.cpp
    mgr.createDirectoryOnDisk("TITI");
    const std::string& exisitng_path      = mgr.getCurrentDir() + IGFD::Utils::GetPathSeparator() + "TITI";
    const std::string& existing_path_file = exisitng_path + IGFD::Utils::GetPathSeparator() + "toto.cpp";
    mgr.createFileOnDisk(existing_path_file);  // we crate also a local file toto.cpp in the dir TITI
    mgr.setInput("toto.cpp");                  // we select the file toto.cpp
    mgr.setCurrentPath(exisitng_path);         // in the dir TITI
    // but we must ensure than the returned file path is TITI/toto.cpp and not the toto.cpp from the current dir
    if (fd.fileManager.GetResultingFilePathName(fd, IGFD_ResultMode_AddIfNoFileExt) != existing_path_file) return false;
    return true;
}

#pragma endregion

#pragma region Entry Point

#define IfTestExist(v) \
    if (vTest == std::string(#v)) return v()

bool Test_FileManager(const std::string& vTest) {
    IfTestExist(Test_IGFD_FileManager_Filtering_asterisk_0);
    IfTestExist(Test_IGFD_FileManager_Filtering_asterisk_1);
    IfTestExist(Test_IGFD_FileManager_Filtering_asterisk_2);
    IfTestExist(Test_IGFD_FileManager_Filtering_asterisk_3);
    IfTestExist(Test_IGFD_FileManager_Filtering_divers_0);
    IfTestExist(Test_IGFD_FileManager_Filtering_divers_1);
    IfTestExist(Test_IGFD_FileManager_Filtering_divers_2);
    IfTestExist(Test_IGFD_FileManager_Filtering_sensitive_case_0);
    IfTestExist(Test_IGFD_FileManager_Filtering_sensitive_case_1);
    IfTestExist(Test_IGFD_FileManager_Filtering_insensitive_case_0);
    IfTestExist(Test_IGFD_FileManager_Filtering_insensitive_case_1);
    IfTestExist(Test_IGFD_FileManager_FileInfos_SearchForExts_0);
    IfTestExist(Test_IGFD_FileManager_GetResultingFilePathName_issue_144);
    IfTestExist(Test_IGFD_FileManager_GetResultingFilePathName_issue_184);
    
    assert(0);

    return false;
}

#pragma endregion
