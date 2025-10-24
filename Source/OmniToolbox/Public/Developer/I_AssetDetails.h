// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_AssetDetails.generated.h"

// This class does not need to be modified.
UINTERFACE()
class OMNITOOLBOX_API UI_AssetDetails : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};
inline UI_AssetDetails::UI_AssetDetails(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

/**
 * This interface is required for a class to be automatically added
 * to the content browsers context menu
 * TODO: Add blueprint support
 * TODO: Add function for getting documentation URL, then add button through CommonActions extension menu to open URL
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
	FString GetDocumentationURL() const;
};
