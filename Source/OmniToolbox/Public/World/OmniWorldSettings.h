// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "OmniWorldSettings.generated.h"

class UOmniWorldAction;
/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API AOmniWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "GameMode")
	TArray<TSoftObjectPtr<UOmniWorldAction>> WorldActions;
};
