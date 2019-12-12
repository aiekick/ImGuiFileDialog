# ImGuiFileDialog

An example of the File Dialog integrated within the ImGui Demo App

- separate system for call and display
- can use custom pane via function binding
  - this pane can block the validation of the dialog
  - can also display different things according to current filter
- support of Filter Coloring
- compatible with MacOs, Linux, Win
  - On Win version you can list Drives
  
## Simple Dialog :
```cpp
void drawGui()
{ 
  // open Dialog Simple
  if (ImGui::Button("Open File Dialog"))
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp\0.h\0.hpp\0\0", ".");

  // display
  if (ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey")) 
  {
    // action if OK
    if (ImGuiFileDialog::Instance()->IsOk == true)
    {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // action
    }
    // close
    ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
  }
}
```
![alt text](dlg_simple.gif)

## Dialog with Custom Pane :
```cpp
static bool canValidateDialog = false;
inline void InfosPane(std::string vFilter, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
	ImGui::Text("Selected Filter : %s", vFilter.c_str());
	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
	if (vCantContinue)
	    *vCantContinue = canValidateDialog;
}

void drawGui()
{
  // open Dialog with Pane
  if (ImGui::Button("Open File Dialog with a custom pane"))
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp\0.h\0.hpp\0\0",
            ".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2), 350, "InfosPane");

  // display and action if ok
  if (ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey")) 
  {
    if (ImGuiFileDialog::Instance()->IsOk == true)
    {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // action
    }
    // close
    ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
  }
}
```
![alt text](dlg_with_pane.gif)

## Filter Coloring

```cpp
ImGuiFileDialog::Instance()->SetFilterColor(".cpp", ImVec4(1,1,0,0.5));
ImGuiFileDialog::Instance()->SetFilterColor(".h", 	ImVec4(0,1,0,0.5));
ImGuiFileDialog::Instance()->SetFilterColor(".hpp", ImVec4(0,0,1,0.5));
ImGuiFileDialog::Instance()->SetFilterColor(".md", 	ImVec4(1,0,1,0.5));
ImGuiFileDialog::Instance()->SetFilterColor(".png", ImVec4(0,1,1,0.5));
```

![alt text](color_filter.png)

## Compilation Ok for Win / Linux / MacOs

Win Version :
![alt text](win.png)

Linux Version :
![alt text](linux.png)

MacOs Version :
![alt text](macos.png)
