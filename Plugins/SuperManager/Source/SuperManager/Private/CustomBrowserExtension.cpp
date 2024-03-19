#include "CustomBrowserExtension.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "ContentBrowserModule.h"
#include "CustomLogger.h"
#include "EditorAssetLibrary.h"
#include "MyEditorUtils.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "SlateWidgets/SlateAdvanceDeletionWidget.h"

void FCustomBrowserExtension::InitContentBrowserExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	
	TArray<FContentBrowserMenuExtender_SelectedPaths>& PathViewContextMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	
	PathViewContextMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FCustomBrowserExtension::CustomContentBrowserExtender));
	
}

void FCustomBrowserExtension::InitCustomEditorFunctionality()
{
	FTabSpawnerEntry& TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FCustomBrowserExtension::OnCreateAdvanceDeletionTab));

	TabSpawnerEntry.SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<FExtender> FCustomBrowserExtension::CustomContentBrowserExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShared<FExtender>();

	if(SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension( FName("Delete"),
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FCustomBrowserExtension::AddContentBrowserExtensionOptions)
		);
		
		SelectedFolders = SelectedPaths;
	}
	

	return MenuExtender;
}

void FCustomBrowserExtension::AddContentBrowserExtensionOptions(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely deletes all unused assets in selected folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FCustomBrowserExtension::OnDeleteUnusedAssetButtonClicked)
		);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Deletes all empty folders selected recursively")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FCustomBrowserExtension::OnDeleteEmptyFoldersClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("Displays all assets in selected folder with some additional functionalities")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FCustomBrowserExtension::OnAdvanceDeletionClicked)
	);
}

void FCustomBrowserExtension::OnDeleteUnusedAssetButtonClicked()
{
	if(SelectedFolders.Num() == 0)
		return;
	
	if(SelectedFolders.Num() > 1)
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("Multiple Folders Selected, please select only 1"), true);
		return;
	}

	TArray<FString> PathNames = UEditorAssetLibrary::ListAssets(SelectedFolders[0], true);

	if(PathNames.Num() == 0)
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("No asset found in selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = CustomLogger::ShowMessageDialog(EAppMsgType::YesNo, FString::Printf(TEXT("Found %d assets in folder\nWould you like to proceed?"), PathNames.Num()));

	if(ConfirmResult == EAppReturnType::No)
		return;

	FMyEditorUtils::FixRedirectors();
	
	TArray<FAssetData> UnusedAssetsList;

	for (const FString& PathName : PathNames)
	{
		if(PathName.Contains(TEXT("Developers")) || PathName.Contains(TEXT("Collections")) || PathName.StartsWith(TEXT("__")))
			continue;

		if(!UEditorAssetLibrary::DoesAssetExist(PathName))
			continue;

		const TArray<FString> Referencers =  UEditorAssetLibrary::FindPackageReferencersForAsset(PathName);

		if(Referencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(PathName);

			UnusedAssetsList.Add(UnusedAssetData);
		}
		
	}

	if(UnusedAssetsList.Num() > 0)
	{
		const int NumberOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsList, true);

		if(NumberOfAssetsDeleted == 0)
			return;

		CustomLogger::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(NumberOfAssetsDeleted) + TEXT(" unused assets"));
	}
	else
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unused asset found in selected folder"));
	}
	
}

void FCustomBrowserExtension::OnDeleteEmptyFoldersClicked()
{
	if(SelectedFolders.Num() == 0)
		return;
	
	if(SelectedFolders.Num() > 1)
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("Multiple Folders Selected, please select only 1"), true);
		return;
	}

	FMyEditorUtils::FixRedirectors();
	TArray<FString> PathNames = UEditorAssetLibrary::ListAssets(SelectedFolders[0], true, true);

	FString EmptyFolderPathNames = "";
	TArray<FString> EmptyFoldersPathsList;
	int Counter = 0;

	for (const FString& PathName : PathNames)
	{
		if(PathName.Contains(TEXT("Developers")) || PathName.Contains(TEXT("Collections")) || PathName.StartsWith(TEXT("__")))
			continue;

		if(!UEditorAssetLibrary::DoesDirectoryExist(PathName))
			continue;

		if(UEditorAssetLibrary::DoesDirectoryHaveAssets(PathName))
			continue;

		EmptyFolderPathNames.Append(PathName).Append(TEXT("\n"));
		EmptyFoldersPathsList.Add(PathName);
	}

	if(!EmptyFoldersPathsList.Num() == 0)
	{
		EAppReturnType::Type ResponseType = 
		CustomLogger::ShowMessageDialog(EAppMsgType::OkCancel, TEXT("Following empty folders found:\n") + EmptyFolderPathNames + "\nWould you like to delete all?");

		if(ResponseType == EAppReturnType::Cancel)
			return;

		for (const FString& EmptyFolder : EmptyFoldersPathsList)
		{
			if(UEditorAssetLibrary::DeleteDirectory(EmptyFolder))
				++Counter;
			else
				CustomLogger::Print(TEXT("Failed to delete ") + EmptyFolder, FColor::Red);
		}

		if(Counter > 0)
			CustomLogger::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + " empty folders");
	}
	else
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("No empty folders found under selected folder"), false);
	}
}

TSharedRef<SDockTab> FCustomBrowserExtension::OnCreateAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto DockTab = SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		SNew(SAdvanceDeletionTab)
		.AssetsDataToStore(GetAllAssetsUnderSelectedFolder())
	];

	return DockTab;
}

void FCustomBrowserExtension::OnAdvanceDeletionClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

TArray<TSharedPtr<FAssetData>> FCustomBrowserExtension::GetAllAssetsUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> SelectedAssets;

	if(SelectedFolders.Num() == 0 )
	{
		CustomLogger::ShowMessageDialog(EAppMsgType::Ok, TEXT("No folder selected"), true);
		return SelectedAssets;
	}

	TArray<FString> SelectedAssetPaths = UEditorAssetLibrary::ListAssets(SelectedFolders[0]);

	for (const FString& PathName : SelectedAssetPaths)
	{
		if(PathName.Contains(TEXT("Developers")) || PathName.Contains(TEXT("Collections")) || PathName.StartsWith(TEXT("__")))
			continue;

		if(!UEditorAssetLibrary::DoesAssetExist(PathName))
			continue;

		const FAssetData& AssetData = UEditorAssetLibrary::FindAssetData(PathName);

		SelectedAssets.Add(MakeShared<FAssetData>(AssetData));
			
	}

	return SelectedAssets;
}

