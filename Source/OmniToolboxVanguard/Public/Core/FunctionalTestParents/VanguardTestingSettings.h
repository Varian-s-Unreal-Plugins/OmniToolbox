// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VisualLogger/VisualLoggerBinaryFileDevice.h"
#include "VanguardTestingSettings.generated.h"

USTRUCT()
struct FVanguardScalabilityPerfTestSettings
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	float HitchDeltaThreshold = 10;
	
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	float TargetGameThreadTime = 16.66;
};

/**
 * 
 */
UCLASS(Config = EditorPerProjectUserSettings)
class OMNITOOLBOXVANGUARD_API UVanguardTestingSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	/**Should every test be labeled with a tag?
	 * TODO: Implement this */
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	bool EnforceTagsOnAllTests = true;
	
	/**When the tests are being run, we check on tick the delta between
	 * the last frame and the current frame. If the delta exceeds
	 * this amount, then we increment the "Hitches" section in
	 * the performance spreadsheet. */
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	float HitchDeltaThreshold = 10;
	
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	float TargetGameThreadTime = 16.66;
	
	UPROPERTY(Category = "Functional Test", EditAnywhere, Config)
	FVanguardScalabilityPerfTestSettings ScalabilityPerfTestSettings[5];

	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins");
	}
};

// class FVanguardVisualLogDevice : public FVisualLoggerBinaryFileDevice
// {
// 	
// };