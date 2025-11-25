// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"

#include "OmniAssetValidator.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOXVALIDATION_API UOmniAssetValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
public:
	
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	/**This is the function that children of UOmniAssetValidator should override instead
	 * of the regular CanValidateAsset */
	virtual bool OmniValidatorCanValidateAsset(const FAssetData& InAssetData, UObject* InObject,
	                                           FDataValidationContext& InContext) const
	{
		return false;
	}
};
