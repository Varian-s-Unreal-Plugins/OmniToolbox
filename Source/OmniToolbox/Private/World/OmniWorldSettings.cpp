// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "World/OmniWorldSettings.h"

#include "GameFeaturesSubsystem.h"
#include "World/OmniWorldAction.h"

void AOmniWorldSettings::BeginPlay()
{
	Super::BeginPlay();

	for(auto& CurrentWorldAction : WorldActions)
	{
		if(CurrentWorldAction.IsNull())
		{
			continue;
		}
		
		FGameFeatureActivatingContext Context;
		//Only apply to our specific world context if set
		if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		//Execute the actions
		for(auto& CurrentAction : Cast<UOmniWorldAction>(CurrentWorldAction.LoadSynchronous())->Actions)
		{
			if(!CurrentAction)
			{
				continue;
			}

			CurrentAction->OnGameFeatureRegistering();
			CurrentAction->OnGameFeatureActivating(Context);
		}
	}
}

void AOmniWorldSettings::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(!HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
	{
		Super::EndPlay(EndPlayReason);
		return;
	}
	
	FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { });

	for(auto& CurrentWorldAction : WorldActions)
	{
		if(CurrentWorldAction.IsNull())
		{
			continue;
		}
		//Only apply to our specific world context if set
		if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}
	
		//Execute the actions
		for(auto& CurrentAction : Cast<UOmniWorldAction>(CurrentWorldAction.LoadSynchronous())->Actions)
		{
			if(!CurrentAction)
			{
				continue;
			}
	
			CurrentAction->OnGameFeatureDeactivating(Context);
			CurrentAction->OnGameFeatureUnregistering();
		}
	}

	Super::EndPlay(EndPlayReason);
}
