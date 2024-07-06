#include "ImGuiFileDialog/Test_ImGuiFileDialog.h"
#include <string>

// specific
#ifdef WIN32
#include <direct.h>  // _chdir
#define GetCurrentDir _getcwd
#define SetCurrentDir _chdir
#define ROOT_SLASH "\\"
#elif defined(UNIX) 
#include<unistd.h> // chdir
#define GetCurrentDir getcwd
#define SetCurrentDir chdir
#define ROOT_SLASH "/"
#endif

int main(int argc, char** argv) {
    // set current dir when we create files for testing
    std::string root_path = argv[0];
    const auto& pos       = root_path.find_last_of(ROOT_SLASH);
    if (pos != std::string::npos) {
        root_path = root_path.substr(0, pos);
    }
    SetCurrentDir(root_path.c_str());

    // exec a specific test for ctest
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_ImGuiFileDialog(argv[1]) ? 0 : 1;
    }

    return Test_ImGuiFileDialog("Test_IGFD_Utils_NaturalCompare_2") ? 0 : 1;
    
    return 0;
}