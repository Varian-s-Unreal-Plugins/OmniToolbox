// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "AssetValidators/OAV_NamingConvention.h"

#include "OmniValidationSettings.h"
#include "Misc/DataValidation.h"
#include "Parents/OmniStandardizedNamingSchema.h"

bool UOAV_NamingConvention::OmniValidatorCanValidateAsset(const FAssetData& InAssetData, UObject* InObject,
                                                          FDataValidationContext& InContext) const
{
	const UOmniValidationSettings* Settings = GetDefault<UOmniValidationSettings>();
	
	UClass* ObjectClass = InObject->GetClass();
	UBlueprint* Blueprint = Cast<UBlueprint>(InObject);
	if(Blueprint != nullptr)
	{
		ObjectClass = Blueprint->ParentClass;
	}
	
	if(Settings->NamingSchema.IsNull() == false)
	{
		for(auto& Restriction : Settings->NamingSchema.LoadSynchronous()->NamingRestrictions)
		{
			if(ObjectClass->IsChildOf(Restriction.Key) || ObjectClass == Restriction.Key)
			{
				return true;
			}
		}
	}
	
	return false;
}

EDataValidationResult UOAV_NamingConvention::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	const UOmniValidationSettings* Settings = GetDefault<UOmniValidationSettings>();
	UOmniStandardizedNamingSchema* NamingSchema = Settings->NamingSchema.LoadSynchronous();
	FOmniNamingSchemaData NamingData;
	UClass* ObjectClass = InAsset->GetClass();
	UBlueprint* Blueprint = Cast<UBlueprint>(InAsset);
	if(Blueprint != nullptr)
	{
		ObjectClass = Blueprint->ParentClass;
	}
	for(auto& Restriction : Settings->NamingSchema->NamingRestrictions)
	{
		if(ObjectClass->IsChildOf(Restriction.Key) || ObjectClass == Restriction.Key)
		{
			NamingData = Restriction.Value;
			break;
		}
	}
	
	
	FString AssetName = InAssetData.AssetName.ToString();
	
	if(NamingData.Prefix.IsEmpty() == false && AssetName.StartsWith(NamingData.Prefix) == false)
	{
		Context.AddError(FText::FromString(FString::Printf(
			TEXT("Asset %s does not match the naming standard. Missing %s prefix"), *InAssetData.AssetName.ToString(), *NamingData.Prefix)));
	}
	
	if(NamingData.Suffix.IsEmpty() == false && AssetName.EndsWith(NamingData.Suffix) == false)
	{
		Context.AddError(FText::FromString(FString::Printf(
	TEXT("Asset %s does not match the naming standard. Missing %s suffix"), *InAssetData.AssetName.ToString(), *NamingData.Suffix)));
	}
	
	return EDataValidationResult::Valid;
}
