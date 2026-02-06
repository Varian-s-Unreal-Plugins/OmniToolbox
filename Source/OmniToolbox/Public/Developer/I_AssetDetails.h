// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_AssetDetails.generated.h"

USTRUCT(Blueprintable)
struct FOmniDocumentationLinkAndText
{
	FOmniDocumentationLinkAndText() = default;

	FOmniDocumentationLinkAndText(const FString& URL, const FString& Text)
		: URL(URL),
		  Text(Text)
	{
	}

	explicit FOmniDocumentationLinkAndText(const FString& URL)
		: URL(URL)
	{
	}

	GENERATED_BODY()

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
	FString URL;

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
	FString Text;
};

// This class does not need to be modified.
UINTERFACE()
class OMNITOOLBOX_API UI_AssetDetails : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};
inline UI_AssetDetails::UI_AssetDetails(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

namespace OmniToolbox
{
	static FLinearColor BlueprintAssetColor = FLinearColor(FColor( 63, 126, 255 ));
}

/**
 * This interface is required for a class to be automatically added
 * to the content browsers context menu
 */
class OMNITOOLBOX_API II_AssetDetails
{
	GENERATED_BODY()

public:

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	bool AppearsInContextMenu() const;

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	TArray<FText> GetAssetsCategories() const;

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	FLinearColor GetAssetColor() const;

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	bool UseChildrenCreationWindow() const;

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	UTexture2D* GetThumbnail() const;

	UFUNCTION(Category = "OmniToolbox|Asset Details", BlueprintNativeEvent)
	TArray<FOmniDocumentationLinkAndText> GetDocumentationLinks() const;
};
