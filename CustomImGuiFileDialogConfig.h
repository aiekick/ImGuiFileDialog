#pragma once

//#include <imgui.h>
#include <CustomFont.h>

// uncomment and modify defines under for customize ImGuiFileDialog

//this options need c++17
// in this app its defined in CMakeLists.txt
//#define USE_STD_FILESYSTEM
 
//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

// the slash's buttons in path cna be used for quick select parallles directories
#define USE_QUICK_PATH_SELECT

// the spacing between button path's can be customized. 
// if disabled the spacing is defined by the imgui theme
// define the space between path buttons 
//#define CUSTOM_PATH_SPACING 2

#define USE_THUMBNAILS
//the thumbnail generation use the stb_image and stb_resize lib who need to define the implementation
//btw if you already use them in your app, you can have compiler error due to "implementation found in double"
//so uncomment these line for prevent the creation of implementation of these libs again in ImGuiFileDialog
//#define DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
//#define DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
//#define IMGUI_RADIO_BUTTON RadioButton
//#define DisplayMode_ThumbailsList_ImageHeight 32.0f
//#define tableHeaderFileThumbnailsString " Thumbnails"
#define DisplayMode_FilesList_ButtonString ICON_IGFD_FILE_LIST
//#define DisplayMode_FilesList_ButtonHelp "File List"
#define DisplayMode_ThumbailsList_ButtonString ICON_IGFD_FILE_LIST_THUMBNAILS
//#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
#define DisplayMode_ThumbailsGrid_ButtonString ICON_IGFD_FILE_GRID_THUMBNAILS
//#define DisplayMode_ThumbailsGrid_ButtonHelp "Thumbnails Grid"
// sleep time in millisedonce to apply in the thread loop, when no datas to extract
// the sleep time can be ssen jsut after the opening of a directory who contain pictures
#define DisplayMode_ThumbailsGrid_ThreadSleepTimeInMS 50

//#define USE_EXPLORATION_BY_KEYS
// Up key for explore to the top
#define IGFD_KEY_UP ImGuiKey_UpArrow
// Down key for explore to the bottom
#define IGFD_KEY_DOWN ImGuiKey_DownArrow
// Enter key for open directory
#define IGFD_KEY_ENTER ImGuiKey_Enter
// BackSpace for comming back to the last directory
#define IGFD_KEY_BACKSPACE ImGuiKey_Backspace

// by ex you can quit the dialog by pressing the key excape
#define USE_DIALOG_EXIT_WITH_KEY
#define IGFD_EXIT_KEY ImGuiKey_Escape

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standard button
//#define IMGUI_BUTTON ImGui::Button

// locales string
#define createDirButtonString ICON_IGFD_ADD
#define resetButtonString ICON_IGFD_RESET
#define drivesButtonString ICON_IGFD_DRIVES
#define editPathButtonString ICON_IGFD_EDIT
#define searchString ICON_IGFD_SEARCH
#define dirEntryString ICON_IGFD_FOLDER
#define linkEntryString ICON_IGFD_LINK
#define fileEntryString ICON_IGFD_FILE
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonEditPathString "Edit path\nYou can also right click on path buttons"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#define OverWriteDialogConfirmButtonString ICON_IGFD_OK " Confirm"
#define OverWriteDialogCancelButtonString ICON_IGFD_CANCEL " Cancel"

//Validation buttons
#define okButtonString ICON_IGFD_OK " OK"
#define okButtonWidth 100.0f
#define cancelButtonString ICON_IGFD_CANCEL " Cancel"
#define cancelButtonWidth 100.0f
//alignement [0:1], 0.0 is left, 0.5 middle, 1.0 right, and other ratios
#define okCancelButtonAlignement 0.0f
//#define invertOkAndCancelButtons 0

// see strftime functionin <ctime> for customize
// "%Y/%m/%d %H:%M:%S" give 2021:01:22 11:47:10
// "%Y/%m/%d %i:%M%p" give 2021:01:22 11:45PM
//#define DateTimeFormat "%Y/%m/%d %i:%M%p"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#define tableHeaderAscendingIcon ICON_IGFD_CHEVRON_UP
#define tableHeaderDescendingIcon ICON_IGFD_CHEVRON_DOWN
//#define tableHeaderFileNameString " File name"
//#define tableHeaderFileTypeString " Type"
//#define tableHeaderFileSizeString " Size"
//#define tableHeaderFileDateString " Date"

#define USE_BOOKMARK
//#define bookmarkPaneWith 150.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
#define bookmarksButtonString ICON_IGFD_BOOKMARK
//#define bookmarksButtonHelpString "bookmark"
#define addBookmarkButtonString ICON_IGFD_ADD
#define removeBookmarkButtonString ICON_IGFD_REMOVE
