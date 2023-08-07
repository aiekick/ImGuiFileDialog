#include "ImGuiFileDialog/Test_ImGuiFileDialog.h"
#include <string>

int main(int argc, char** argv) {
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_ImGuiFileDialog(argv[1]) ? 0 : 1;
    }

    return Test_ImGuiFileDialog("Test_IGFD_Utils_ReplaceString_3") ? 0 : 1;
    
    return 0;
}