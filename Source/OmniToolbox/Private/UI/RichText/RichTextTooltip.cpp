// Copyright (C) Varian Daemon 2023. All Rights Reserved.


#include "UI/RichText/RichTextTooltip.h"
#include "Widgets/SWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/STooltip.h"

// Class charged of creating the inline tooltip
class FRichTextInlineTooltip : public FRichTextDecorator
{
public:
	FRichTextInlineTooltip(URichTextBlock* InOwner, const FTextBlockStyle& InTextStyle, const FTextBlockStyle& InTooltipTextStyle, const TSubclassOf<UUserWidget>& ToolTipWidget)
		: FRichTextDecorator(InOwner)
		, TextStyle(InTextStyle)
		, TooltipTextStyle(InTooltipTextStyle)
	{
	}

	// Only valid if text is: <tooltip text="Some infos">Some text</>
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		// return RunParseResult.Name == TEXT("tooltip");
		return RunParseResult.Name == TEXT("tooltip") && RunParseResult.MetaData.Contains(TEXT("text"));
	}
	
	/**
	* Create a STextBlock with a tooltip text.
	* 
	* For <tooltip text="Some infos">Some text</>:
	* - RunInfo.Content is "Some text"
	* - RunInfo.MetaData[TEXT("text")] is "Some infos"
	*/
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& InRunInfo, const FTextBlockStyle& InTextStyle) const override
	{
		return SNew(STextBlock)
			.Text(InRunInfo.Content)
			.TextStyle(&TextStyle)
			.ToolTip(SNew(SToolTip)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InRunInfo.MetaData[TEXT("text")]))
				.TextStyle(&TooltipTextStyle)
			]);
	}
	
	FTextBlockStyle TextStyle;
	FTextBlockStyle TooltipTextStyle;
};

URichTextTooltip::URichTextTooltip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedPtr<ITextDecorator> URichTextTooltip::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FRichTextInlineTooltip(InOwner, TextStyle, TooltipTextStyle, TooltipWidget));
}
