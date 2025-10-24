// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "Blueprint/UserWidget.h"
#include "RichTextTooltip.generated.h"

/**
 * V: UNFINISHED! Do not use! (most likely doesn't work anyway)
 */
UCLASS(Abstract, Blueprintable)
class OMNITOOLBOX_API URichTextTooltip : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:

	URichTextTooltip(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

protected:
	
	// Style used for text
	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle TextStyle;

	// Style used for tooltip text
	UPROPERTY(EditAnywhere, Category=Appearance)
	FTextBlockStyle TooltipTextStyle;

	UPROPERTY(EditAnywhere, Category=Appearance)
	TSubclassOf<UUserWidget> TooltipWidget = nullptr;
};
