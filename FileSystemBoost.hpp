#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <boost/filesystem.hpp>
#include <iostream>

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#define _IGFD_WIN_
#include <Windows.h>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)
#define _IGFD_UNIX_
#endif

namespace fs = boost::filesystem;

class FileSystemBoost : public IGFD::IFileSystem {
public:
public:
    bool IsDirectoryCanBeOpened(const std::string& vName) override {
        bool bExists = false;
        if (!vName.empty()) {
#ifdef _IGFD_WIN_
            std::wstring wname = IGFD::Utils::UTF8Decode(vName.c_str());
            fs::path pathName  = fs::path(wname);
#else   // _IGFD_WIN_
            fs::path pathName = fs::path(vName);
#endif  // _IGFD_WIN_
            try {
                // interesting, in the case of a protected dir or for any reason the dir cant be opened
                // this func will work but will say nothing more . not like the dirent version
                bExists = fs::is_directory(pathName);
                // test if can be opened, this function can thrown an exception if there is an issue with this dir
                // here, the dir_iter is need else not exception is thrown..
                const auto dir_iter = fs::directory_iterator(pathName);
                (void)dir_iter;  // for avoid unused warnings
            } catch (const std::exception& /*ex*/) {
                // fail so this dir cant be opened
                bExists = false;
            }
        }
        return bExists;  // this is not a directory!
    }
    bool IsDirectoryExist(const std::string& vName) override {
        if (!vName.empty()) {
#ifdef _IGFD_WIN_
            std::wstring wname = IGFD::Utils::UTF8Decode(vName.c_str());
            fs::path pathName  = fs::path(wname);
#else   // _IGFD_WIN_
            fs::path pathName = fs::path(vName);
#endif  // _IGFD_WIN_
            return fs::is_directory(pathName);
        }
        return false;  // this is not a directory!
    }
    bool IsFileExist(const std::string& vName) override {
        return fs::is_regular_file(vName);
    }
    bool CreateDirectoryIfNotExist(const std::string& vName) override {
        bool res = false;
        if (!vName.empty()) {
            if (!IsDirectoryExist(vName)) {
#ifdef _IGFD_WIN_
                std::wstring wname = IGFD::Utils::UTF8Decode(vName.c_str());
                fs::path pathName  = fs::path(wname);
                res                = fs::create_directory(pathName);
#elif defined(__EMSCRIPTEN__)  // _IGFD_WIN_
                std::string str = std::string("FS.mkdir('") + vName + "');";
                emscripten_run_script(str.c_str());
                res = true;
#elif defined(_IGFD_UNIX_)
                char buffer[PATH_MAX] = {};
                snprintf(buffer, PATH_MAX, "mkdir -p \"%s\"", vName.c_str());
                const int dir_err = std::system(buffer);
                if (dir_err != -1) {
                    res = true;
                }
#endif  // _IGFD_WIN_
                if (!res) {
                    std::cout << "Error creating directory " << vName << std::endl;
                }
            }
        }
        return res;
    }
    std::vector<std::string> GetDrivesList() override {
        std::vector<std::string> res;
#ifdef _IGFD_WIN_
        const DWORD mydrives = 2048;
        char lpBuffer[2048];
#define mini(a, b) (((a) < (b)) ? (a) : (b))
        const DWORD countChars = mini(GetLogicalDriveStringsA(mydrives, lpBuffer), 2047);
#undef mini
        if (countChars > 0U && countChars < 2049U) {
            std::string var = std::string(lpBuffer, (size_t)countChars);
            IGFD::Utils::ReplaceString(var, "\\", "");
            res = IGFD::Utils::SplitStringToVector(var, '\0', false);
        }
#endif  // _IGFD_WIN_
        return res;
    }

    IGFD::Utils::PathStruct ParsePathFileName(const std::string& vPathFileName) override {
        IGFD::Utils::PathStruct res;
        if (vPathFileName.empty()) return res;
        auto fsPath = fs::path(vPathFileName);
        if (fs::is_directory(fsPath)) {
            res.name = "";
            res.path = fsPath.string();
            res.isOk = true;
        } else if (fs::is_regular_file(fsPath)) {
            res.name = fsPath.filename().string();
            res.path = fsPath.parent_path().string();
            res.isOk = true;
        }
        return res;
    }

    std::vector<IGFD::FileInfos> ScanDirectory(const std::string& vPath) override {
        std::vector<IGFD::FileInfos> res;
        try {
            const fs::path fspath(vPath);
            IGFD::FileType fstype = IGFD::FileType(IGFD::FileType::ContentType::Directory, fs::is_symlink(fs::status(fspath)));
            {
                IGFD::FileInfos file_two_dot;
                file_two_dot.filePath    = vPath;
                file_two_dot.fileNameExt = "..";
                file_two_dot.fileType    = fstype;
                res.push_back(file_two_dot);
            }
            const auto& dir_iter = fs::directory_iterator(fspath);
            for (const auto& file : dir_iter) {
                IGFD::FileType fileType;
                if (file.is_symlink()) {
                    fileType.SetSymLink(file.is_symlink());
                    fileType.SetContent(IGFD::FileType::ContentType::LinkToUnknown);
                }
                if (file.is_directory()) {
                    fileType.SetContent(IGFD::FileType::ContentType::Directory);
                }  // directory or symlink to directory
                else if (file.is_regular_file()) {
                    fileType.SetContent(IGFD::FileType::ContentType::File);
                }
                if (fileType.isValid()) {
                    auto fileNameExt = file.path().filename().string();
                    {
                        IGFD::FileInfos _file;
                        _file.filePath    = vPath;
                        _file.fileNameExt = fileNameExt;
                        _file.fileType    = fileType;
                        res.push_back(_file);
                    }
                }
            }
        } catch (const std::exception& ex) {
            std::cout << ex.what() << std::endl;
        }
        return res;
    }
    bool IsDirectory(const std::string& vFilePathName) override {
        return fs::is_directory(vFilePathName);
    }
};

#define FILE_SYSTEM_OVERRIDE FileSystemBoost
