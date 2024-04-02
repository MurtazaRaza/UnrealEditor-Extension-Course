#pragma once
#include "Widgets/SCompoundWidget.h"

class SlateAdvanceDeletionWidget
{
public:
	
};

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}

	SLATE_ARGUMENT(TArray<TSharedPtr<struct FAssetData>>, AssetsDataToStore)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& Arguments);

private:
	TArray<TSharedPtr<struct FAssetData>> SelectedAssetList;
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;

	TSharedRef<ITableRow> OnListItemGenerated(TSharedPtr<struct FAssetData> AssetData, const TSharedRef<STableViewBase>& TableViewBase);
	TSharedRef<SCheckBox> ConstructCheckbox(const TSharedPtr<struct FAssetData>& AssetData);
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);
	void OnCheckStateChanged(ECheckBoxState NewState, TSharedPtr<struct FAssetData> AssetData);

	FSlateFontInfo GetEmbossedTextFont();
};
