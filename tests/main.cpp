#include "ImGuiFileDialog/Test_ImGuiFileDialog.h"
#include <string>

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		printf("Exec test : %s\n", argv[1]);
		return Test_ImGuiFileDialog(argv[1]) ? 0 : 1;
	}

    //return Test_ImGuiFileDialog("IGFD::FilterManager::ParseFilters::Filters::Simple::1") ? 0 : 1;
    //return Test_ImGuiFileDialog("IGFD::FilterManager::ParseFilters::Filters::Collection::2") ? 0 : 1;
    //return Test_ImGuiFileDialog("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::8") ? 0 : 1;
    //return Test_ImGuiFileDialog("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::1") ? 0 : 1;
    return Test_ImGuiFileDialog("IGFD::FilterManager::ParseFilters::Filters::Divers::3") ? 0 : 1;

	return 0;
}