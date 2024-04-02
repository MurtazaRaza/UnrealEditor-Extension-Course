#include "SuperManager/Public/SlateWidgets/SlateAdvanceDeletionWidget.h"

#include "CustomLogger.h"

FSlateFontInfo SAdvanceDeletionTab::GetEmbossedTextFont()
{
	return FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
}

void SAdvanceDeletionTab::Construct(const FArguments& Arguments)
{
	ThumbnailPool = MakeShareable(new FAssetThumbnailPool(24, false));
	
	bCanSupportFocus = true;

	SelectedAssetList = Arguments._AssetsDataToStore;

	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 20;

	ChildSlot
	[
		// Main vertical box
		SNew(SVerticalBox)
		
		// Title Text
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advance Deletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]
		
		// Dropdown for listing conditions
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
		
		// Asset List
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			// Scroll Box
			SNew(SScrollBox)

			// List View
			+SScrollBox::Slot()
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				.ItemHeight(24.f)
				.ListItemsSource(&SelectedAssetList)
				.OnGenerateRow(this, &SAdvanceDeletionTab::OnListItemGenerated)
			]
		]
		
		// Interaction Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnListItemGenerated(TSharedPtr<FAssetData> AssetData, const TSharedRef<STableViewBase>& TableViewBase)
{
	const FString AssetDataClassName = AssetData->AssetClassPath.ToString();
	const FString AssetName = AssetData->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 7;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 8;

	TSharedPtr<FAssetThumbnail> Thumbnail = MakeShared<FAssetThumbnail>(AssetData.ToSharedRef().Get(), 64, 64, ThumbnailPool);
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> RowElement =
		SNew(STableRow<TSharedPtr<FAssetData>>, TableViewBase)
		[
			SNew(SHorizontalBox)
			
			// Checkbox
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(0.05f)
			[
				ConstructCheckbox(AssetData)
			]
			// Asset Class Name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(0.2f)
			[
				ConstructTextForRowWidget(AssetDataClassName, AssetClassNameFont)
			]

			// Asset Name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(1.f)
			[
				ConstructTextForRowWidget(AssetName, AssetNameFont)
			]

			// Image screenshot
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.FillWidth(.5f)
			[
				SNew(SBox)
				.MaxDesiredHeight(64.f)
				.MaxDesiredWidth(64.f)
				.HAlign(HAlign_Right)
				[
				   Thumbnail.Get()->MakeThumbnailWidget()
				]
				// Thumbnail.Get()->MakeThumbnailWidget()
			]

			// Button
		];

	return RowElement;
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckbox(const TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SCheckBox> Checkbox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckStateChanged, AssetData)
	.Visibility(EVisibility::Visible);

	return Checkbox;
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> RowText = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return RowText;
}

void SAdvanceDeletionTab::OnCheckStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		CustomLogger::Print(AssetData->GetExportTextName() + " Unchecked", FColor::Red);
		break;
	case ECheckBoxState::Checked:
		CustomLogger::Print(AssetData->GetExportTextName() + " Checked", FColor::Green);
		break;
	case ECheckBoxState::Undetermined:
		CustomLogger::Print(AssetData->GetExportTextName() + " Undetermined", FColor::Orange);
		break;
	default: ;
	}
	CustomLogger::Print(AssetData->AssetName.ToString(), FColor::Green);
}
