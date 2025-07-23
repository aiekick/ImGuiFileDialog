# New Api

the idea is to have a imGui api with some pre defined component 
the user can place where he want and add his own widgets easily

no RTTI like ImGui

# ex :

```cpp
ImGuiFileDialog::OpenDialog();

bool ImGuiFileDialog::Display() {
	if (ImGuiFileDialog::BeginDialog()) {
		if (ImGuiFileDialog::BeginHeader()) {
			ImGuiFileDialog::FavoriteButton();
			ImGuiFileDialog::CreateDirectoryButton();
			ImGuiFileDialog::ComeBackButton();
			ImGui::Separator();
			ImGuiFileDialog::PathComposer();
			ImGuiFileDialog::EndHeader();
		}
		if (ImGuiFileDialog::BeginHeader()) {
			ImGuiFileDialog::ViewModeButtons();
			ImGui::Separator();
			ImGuiFileDialog::SearchBar();
			ImGuiFileDialog::EndHeader();
		}
		if (ImGuiFileDialog::BeginContent()) {
			if (ImGuiFileDialog::BeginPane()){
				ImGuiFileDialog::BookmarPane();
				ImGuiFileDialog::PlacesPane();
				ImGuiFileDialog::DevicesPane();
				ImGuiFileDialog::ExplorerPane();
				ImGuiFileDialog::EndPane();
			}
			ImGuiFileDialog::FilesView();
			ImGuiFileDialog::EndContent();
		}
		
		if (ImGuiFileDialog::BeginFooter()) {
			ImGuiFileDialog::FileNameInput();
			ImGuiFileDialog::FilterCombobox();
			ImGuiFileDialog::EndFooter();
		}
		
		if (ImGuiFileDialog::BeginFooter()) {
			ImGuiFileDialog::OkButton();
			ImGuiFileDialog::CancelButton();
			ImGuiFileDialog::EndFooter();
		}	
		
		ImGuiFileDialog::EndDialog();
	}
}

if (ImGuiFileDialog::Display()) {
	if (ImGuiFileDialog::isOk()) {
		auto file = ImGuiFileDialog::getFile();
		auto files = ImGuiFileDialog::getFiles();
		auto path = ImGuiFileDialog::getpath();
	}
	ImGuiFileDialog::close();
}
```