#pragma once

class FCustomBrowserExtension
{

public:
	void InitContentBrowserExtension();
	void InitCustomEditorFunctionality();

private:
	TArray<FString> SelectedFolders;
	
	TSharedRef<FExtender> CustomContentBrowserExtender(const TArray<FString>& SelectedPaths);
	void AddContentBrowserExtensionOptions(class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFoldersClicked();
	void OnAdvanceDeletionClicked();

	TSharedRef<SDockTab> OnCreateAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedPtr<struct FAssetData>> GetAllAssetsUnderSelectedFolder();
};
