// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"	
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"	

TMap<UClass*, FString> UQuickAssetAction::TypeToPrefixMap =
	{
	{UBlueprint::StaticClass(),TEXT("BP_")},
{UStaticMesh::StaticClass(),TEXT("SM_")},
{UMaterial::StaticClass(), TEXT("M_")},
{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
{UParticleSystem::StaticClass(), TEXT("PS_")},
{USoundCue::StaticClass(), TEXT("SC_")},
{USoundWave::StaticClass(), TEXT("SW_")},
{UTexture::StaticClass(), TEXT("T_")},
{UTexture2D::StaticClass(), TEXT("T_")},
{UUserWidget::StaticClass(), TEXT("WBP_")},
{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
{UNiagaraSystem::StaticClass(), TEXT("NS_")},
{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
	};


void UQuickAssetAction::DuplicateAssets(int32 NumberOfDuplicates)
{
	if(NumberOfDuplicates <= 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"), true);
		
		return;
	}
	const TArray<FAssetData>& SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int i = 0; i < NumberOfDuplicates; ++i)
		{
			const FString& SourceAssetPath = SelectedAssetData.GetObjectPathString();
			const FString DuplicatedAssetName = FString::Printf(TEXT("%s%s%s"), *SelectedAssetData.AssetName.ToString(), TEXT("_"), *(FString::FromInt(i + 1)));
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), DuplicatedAssetName);

			if(!UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
				continue;
			
			UEditorAssetLibrary::SaveAsset(NewPathName, false);
			++Counter;
		}
	}

	if(Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Asset Duplication SUCCESS for ") +  FString::FromInt(Counter) + " Files");
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	int Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if(!SelectedObject)
			continue;

		FString* PrefixFound = TypeToPrefixMap.Find(SelectedObject->GetClass());
		if(!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Prefix not found for class ") + SelectedObject->GetClass()->GetName(), FColor::Yellow);
			continue;
		}
		
		FString OldName = SelectedObject->GetName();
		if(OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print( OldName + TEXT(" already has prefix added to it "), FColor::Yellow);
			continue;
		}

		const FString NewName = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);

		++Counter;
	}

	DebugHeader::ShowNotifyInfo(TEXT("Prefix added to ") + FString::FromInt(Counter) + " Files");
}

void UQuickAssetAction::DeleteSelectedFiles()
{
	TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssets;

	FixRedirectors();

	for (auto SelectedAsset : SelectedAssets)
	{
		const TArray<FString> SelectedObjectReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAsset.GetObjectPathString());

		if(SelectedObjectReferencers.Num() == 0)
			UnusedAssets.Add(SelectedAsset);
	}

	if(UnusedAssets.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("No unsed assets selected"));
		return;
	}

	const int NumberOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssets, true);

	if(NumberOfAssetsDeleted == 0)
		return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(NumberOfAssetsDeleted) + TEXT(" unused assets"));
}

void UQuickAssetAction::FixRedirectors()
{
	//Load in asset registry module for later use
	IAssetRegistry& AssetRegistry = 
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
 
	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
 
	//Use class path name for searching redirectors
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
 
	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);
 
	if (AssetList.Num() == 0) return;
 
	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}
 
	TArray<UObject*> Objects;
	constexpr bool bAllowedToPromptToLoadAssets = true;
	constexpr bool bLoadRedirects = true;
 
	if (AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, bAllowedToPromptToLoadAssets, bLoadRedirects))
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}
 
		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}
