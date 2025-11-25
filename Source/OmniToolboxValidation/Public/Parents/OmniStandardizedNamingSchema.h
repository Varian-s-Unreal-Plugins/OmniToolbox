// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OmniStandardizedNamingSchema.generated.h"

USTRUCT(BlueprintType)
struct FOmniNamingSchemaData
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
	FString Prefix;
	
	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
	FString Suffix;
};

/**
 * 
 */
UCLASS()
class OMNITOOLBOXVALIDATION_API UOmniStandardizedNamingSchema : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite, meta = (AllowAbstract))
	TMap<TSubclassOf<UObject>, FOmniNamingSchemaData> NamingRestrictions;
};
