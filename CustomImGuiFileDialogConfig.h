#pragma once

// uncomment and modify defines under for customize ImGuiFileDialog

// widget
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standar button
//#define IMGUI_BUTTON ImGui::Button

// locales string
//#define createDirButtonString "+"
//#define okButtonString " OK"
//#define cancelButtonString " Cancel"
//#define resetButtonString "R"
//#define drivesButtonString "Drives"
//#define searchString "Search"
//#define dirEntryString "[DIR] "
//#define linkEntryString "[LINK] "
//#define fileEntryString "[FILE] "
//#define fileNameString "File Name : "
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"

/*
Comment theses line if you not want to have customization, like icon font here
*/

#include <CustomFont.h>

#define createDirButtonString ICON_IMFDLG_ADD
#define okButtonString ICON_IMFDLG_OK " OK"
#define cancelButtonString ICON_IMFDLG_CANCEL " Cancel"
#define resetButtonString ICON_IMFDLG_RESET
#define drivesButtonString ICON_IMFDLG_DRIVES
#define searchString ICON_IMFDLG_SEARCH
#define dirEntryString ICON_IMFDLG_FOLDER
#define linkEntryString ICON_IMFDLG_LINK
#define fileEntryString ICON_IMFDLG_FILE




