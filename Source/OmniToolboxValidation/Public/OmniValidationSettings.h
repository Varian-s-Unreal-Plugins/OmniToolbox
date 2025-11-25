// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OmniValidationSettings.generated.h"

class UOmniStandardizedNamingSchema;
/**
 * 
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "OmniToolbox Validation Settings"))
class OMNITOOLBOXVALIDATION_API UOmniValidationSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	/**Global*/
	UPROPERTY(Category = "", Config, EditAnywhere)
	bool EnableOmniToolboxValidators = true;
	
	UPROPERTY(EditAnywhere, Category = PrimaryAssetRules, meta = (RelativeToGameContentDir, LongPackageName), config)
	TArray<FDirectoryPath> DirectoriesToIgnore;
	
	/**When assigned, OmniToolbox will try to make sure all assets
	 * have a proper naming scheme.
	 * For example, making sure all materials start with an M_ prefix */
	UPROPERTY(Category = "", Config, EditAnywhere)
	TSoftObjectPtr<UOmniStandardizedNamingSchema> NamingSchema;
	
	UPROPERTY(Category = "", Config, EditAnywhere)
	TArray<TSoftClassPtr<UOmniAssetValidator>> ValidatorsToDisable;
	
	
public:
	UFUNCTION()
	static void OpenBlueprintAndFocusNode(UBlueprint* Blueprint, UEdGraph* Graph, UEdGraphNode* Node);
	
	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins");
	}
};
