// From https://github.com/aiekick/ImGuiFileDialog
//
// Modified by AlastairGrowcott@yahoo.com to work on Posix systems.


#ifndef __IMGUI_FILE_DIALOG_H_
#define __IMGUI_FILE_DIALOG_H_

#include <vector>
#include <string>

#define MAX_FILE_DIALOG_NAME_BUFFER  (1024u)

struct FileInfoStruct
{
	char type;
	std::string filePath;
	std::string fileName;
	std::string ext;
};

class ImGuiFileDialog
{
public:
	ImGuiFileDialog();
	~ImGuiFileDialog();

	static ImGuiFileDialog* Instance()
	{
		static ImGuiFileDialog *_instance = new ImGuiFileDialog();
		return _instance;
	}

        // The filters are a null-terminator separated and terminated list of file extensions. Each filter should
        // start with a period. E.g.:
        //     ".txt\0.c\0.cpp\0.h\0"
	bool FileDialog(const char* vName, const char* vFilters = 0, std::string vPath = ".", std::string vDefaultFileName = "");
        void Reset();
	std::string GetFilepathName();
	std::string GetCurrentPath();
	std::string GetCurrentFileName();
	std::string GetCurrentFilter();

        static char FileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER];
	static int FilterIndex;
	bool IsOk;

private:
	std::vector<FileInfoStruct> m_FileList;
	std::string m_SelectedFileName;
	std::string m_CurrentPath;
	std::vector<std::string> m_CurrentPath_Decomposition;
	std::string m_CurrentFilterExt;

	void ScanDir(std::string vPath);
	void SetCurrentDir(std::string vPath);
	void ComposeNewPath(std::vector<std::string>::iterator vIter);
};


#endif // __IMGUI_FILE_DIALOG_H_

