#pragma once

class FCustomBrowserExtension
{

public:
	void InitContentBrowserExtension();
private:
	TArray<FString> SelectedFolders;
	
	TSharedRef<FExtender> CustomContentBrowserExtender(const TArray<FString>& SelectedPaths);
	void AddContentBrowserExtensionOptions(class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFoldersClicked();
};
