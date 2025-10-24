// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClassIconFinder.h"
#include "ContentBrowserModule.h"
#include "OmniToolboxThumbnailRenderer.h"
#include "EditorUtilityWidget.h"
#include "IContentBrowserSingleton.h"
#include "Engine/DeveloperSettings.h"
#include "DS_OmniToolboxSettings.generated.h"

/**
 * 
 */
UCLASS(Config = EditorPerProjectUserSettings, meta = (DisplayName = "OmniToolbox"))
class OMNITOOLBOXEDITOR_API UDS_OmniToolboxSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UDS_OmniToolboxSettings()
	{
		CategoryName = TEXT("Plugins");
		SectionName = TEXT("OmniToolbox");
	}

	/**What classes should have their thumbnails overriden with a custom
	 * texture? This also applies to children of any classes in this map.*/
	UPROPERTY(Category = "Content Browser", config, EditAnywhere)
	TMap<TSoftClassPtr<UObject>, TSoftObjectPtr<UTexture2D>> ThumbnailOverrides;

	/***/
	UPROPERTY(Category = "Content Browser", config, EditAnywhere, meta = (MustImplement = "/Script/OmniToolboxRuntime.I_AssetDetails"))
	TArray<TSoftClassPtr<UObject>> ContextMenuEntries;
};
