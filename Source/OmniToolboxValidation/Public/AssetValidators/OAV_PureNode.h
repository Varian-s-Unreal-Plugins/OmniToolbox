// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Parents/OmniAssetValidator.h"
#include "OAV_PureNode.generated.h"

/**
 * This validator makes sure that most pure nodes aren't connected
 * to multiple nodes, which would cause them to trigger multiple times.
 */
UCLASS()
class OMNITOOLBOXVALIDATION_API UOAV_PureNode : public UOmniAssetValidator
{
	GENERATED_BODY()
	
	virtual bool OmniValidatorCanValidateAsset(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
