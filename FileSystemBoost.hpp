#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <boost/filesystem.hpp>

class FileSystemBoost : public IGFD::IFileSystem {
public:
    bool IsDirectoryCanBeOpened(const std::string& vName) override {
        return false;
    }
    bool IsDirectoryExist(const std::string& vName) override {
        return false;
    }
    bool CreateDirectoryIfNotExist(const std::string& vName) override {
        return false;
    }
    std::vector<std::string> GetDrivesList() override {
        std::vector<std::string> res;
        return res;
    }

    IGFD::Utils::PathStruct ParsePathFileName(const std::string& vPathFileName) override {
        IGFD::Utils::PathStruct res;
        return res;
    }

    std::vector<IGFD::FileInfos> ScanDirectory(const std::string& vPath) override {
        std::vector<IGFD::FileInfos> res;
        return res;
    }
    bool IsDirectory(const std::string& vFilePathName) override {
        return false;
    }
};

#define FILE_SYSTEM_OVERRIDE FileSystemBoost
