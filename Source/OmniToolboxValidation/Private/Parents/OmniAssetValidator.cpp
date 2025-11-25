// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Parents/OmniAssetValidator.h"

#include "OmniValidationSettings.h"

bool UOmniAssetValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                          FDataValidationContext& InContext) const
{
	bool bIsValidatorEnabled = GetDefault<UOmniValidationSettings>()->EnableOmniToolboxValidators;
	
	if(GetDefault<UOmniValidationSettings>()->ValidatorsToDisable.Contains(GetClass()))
	{
		return false;
	}
	
	const FDirectoryPath* FoundPath = GetDefault<UOmniValidationSettings>()->DirectoriesToIgnore.FindByPredicate([InAssetData](FDirectoryPath Path)
	{
		return InAssetData.PackagePath.ToString().Contains(Path.Path);
	});
	
	return bIsValidatorEnabled && OmniValidatorCanValidateAsset(InAssetData, InObject, InContext) && FoundPath == nullptr;
}
