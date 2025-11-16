// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Subsystems/OmniEngineSubsystem.h"

#include "IPlacementModeModule.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH_SVG(StyleSet, RelativePath, ...) \
FSlateVectorImageBrush((StyleSet)->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

void UOmniEngineSubsystem::RegisterClassIcon(const FString& PluginName, const FString& ClassName, const FString& SvgName)
{
#if WITH_EDITOR
	
	//Try to find an existing style set for this plugin
	TSharedPtr<FSlateStyleSet>* FoundStyleSet = StyleSets.Find(PluginName);
	TSharedPtr<FSlateStyleSet> StyleSet;

	if (!FoundStyleSet)
	{
		// Create a new style set for this plugin
		StyleSet = MakeShareable(new FSlateStyleSet(FName(*PluginName)));

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (Plugin.IsValid())
		{
			StyleSet->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OmniStyleSubsystem: Plugin %s not found"), *PluginName);
		}

		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
		StyleSets.Add(PluginName, StyleSet);
	}
	else
	{
		StyleSet = *FoundStyleSet;
	}

	// Set the class icon
	const FName IconName = FName("ClassIcon." + ClassName);
	const FName ThumbnailName = FName("ClassThumbnail." + ClassName);
	StyleSet->Set(IconName, new IMAGE_BRUSH_SVG(StyleSet, SvgName, FVector2D(16)));
	StyleSet->Set(ThumbnailName, new IMAGE_BRUSH_SVG(StyleSet, SvgName, FVector2D(64)));

#endif
}

void UOmniEngineSubsystem::RegisterPlacementPaletteCategory(FString Category, FString DisplayName, FString SVG,
	int32 SortOrder)
{
	FPlacementCategoryInfo CategoryInfo(                                       
	FText::FromString(Category),                                               
	FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(SVG)),                                 
	FName(Category),                                                               
	Category,                                                          
	SortOrder                                                                     
	);                                                                               
	CategoryInfo.ShortDisplayName = FText::FromString(DisplayName);	
	IPlacementModeModule::Get().RegisterPlacementCategory(CategoryInfo);             
}

void UOmniEngineSubsystem::AddClassToPlacementPalette(FString Category, UClass* Class)
{
#if WITH_EDITOR
	const FPlacementCategoryInfo* Info = IPlacementModeModule::Get().GetRegisteredPlacementCategory(FName(Category));
	if(Info)
	{
		IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShareable( new FPlaceableItem(nullptr, FAssetData(Class)) ));
	}
	
#endif
}
