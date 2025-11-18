// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Config/DS_OmniToolboxSettings.h"

#if WITH_EDITOR
#include "IPlacementModeModule.h"
#endif
#include "OmniRuntimeMacros.h"

#if WITH_EDITOR
Omni_OnModuleStarted("OmniToolboxEditor")
{
	for(auto& CurrentClass : GetDefault<UDS_OmniToolboxSettings>()->PlacementPaletteActors)
	{
		if(CurrentClass.Actor.IsNull())
		{
			continue;
		}
		
		const FPlacementCategoryInfo* Info = IPlacementModeModule::Get().GetRegisteredPlacementCategory(CurrentClass.Category);
		if(Info)
		{
			IPlacementModeModule::Get().RegisterPlaceableItem(Info->UniqueHandle, MakeShareable( new FPlaceableItem(nullptr, FAssetData(CurrentClass.Actor.LoadSynchronous())) ));
		}
	}
}
#endif