// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "QuickAssetAction.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SUPERMANAGER_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()

public:

	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumberOfDuplicates);

	UFUNCTION(CallInEditor)
	void AddPrefixes();

	UFUNCTION(CallInEditor)
	void DeleteSelectedFiles();

private:
	
	static TMap<UClass*, FString> TypeToPrefixMap; 
};
