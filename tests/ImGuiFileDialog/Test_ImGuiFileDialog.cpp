#include "Test_ImGuiFileDialog.h"
#include "Test_FilterManager.h"
#include "Test_FileManager.h"

bool Test_ImGuiFileDialog(const std::string& vTest) {
    if (vTest.find("IGFD_FilterManager") != std::string::npos) return Test_FilterManager(vTest);
    else if (vTest.find("IGFD_FileManager") != std::string::npos) return Test_FileManager(vTest);
    return true;
}