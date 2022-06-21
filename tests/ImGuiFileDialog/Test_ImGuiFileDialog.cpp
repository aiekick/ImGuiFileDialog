#include "Test_ImGuiFileDialog.h"
#include "FilterManager/Test_FilterManager.h"

bool Test_ImGuiFileDialog(const std::string& vTest)
{
	if (vTest.find("IGFD::FilterManager") != std::string::npos) return Test_FilterManager(vTest);
	return true;
}