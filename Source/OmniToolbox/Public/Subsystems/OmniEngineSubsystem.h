// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#if WITH_EDITOR
#include "Templates/SharedPointer.h"
#endif
#include "OmniEngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API UOmniEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	//Adds or updates a class icon for a given plugin
	void RegisterClassIcon(const FString& PluginName, const FString& ClassName, const FString& SvgName);
	
	void RegisterPlacementPaletteCategory(FString Category, FString DisplayName, FString SVG, int32 SortOrder);
	void AddClassToPlacementPalette(FString Category, UClass* Class);

private:

#if WITH_EDITOR
	TMap<FString, TSharedPtr<FSlateStyleSet>> StyleSets;
#endif
};
