// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "Engine/DataAsset.h"
#include "UObject/Object.h"
#include "OmniWorldAction.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API UOmniWorldAction : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(Category = "", EditAnywhere, meta = (GetOptions = "OmniToolboxRuntime.OmniHelperLibrary.GetAllGameFeatureNames"))
	TArray<FString> GameFeaturesToEnable;

	UPROPERTY(Category = "", EditAnywhere)
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
