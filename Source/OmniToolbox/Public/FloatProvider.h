// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#if ObjectTags_Enabled
#include "ObjectTags_Subsystem.h"
#endif
#include "UObject/Object.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataTable.h"
#include "StructUtils/InstancedStruct.h"
#include "FloatProvider.generated.h"



/**Base struct for other instanced structs to inherit from.
 * Children structs will need to perform some logic with
 * their own data to evaluate down to a simple float.
 * For example, a struct that holds a runtime float curve
 * or a struct that holds a scalable float.*/
USTRUCT(BlueprintType)
struct FFloatProviderData
{
	GENERATED_BODY()
	FFloatProviderData() = default;
	explicit FFloatProviderData(float BaseValue)
	: BaseValue(BaseValue)	{}
	virtual ~FFloatProviderData() = default;

	/**Many children of this float provider system require a
	 * base float to evaluate their data. Such as FRuntimeFloatCurve. */
	UPROPERTY(Category = "", BlueprintReadWrite)
	float BaseValue = 0;

	/**Some float providers require world context. You'll have to assign
	 * this in whatever object is using this float provider. */
	UPROPERTY(Category = "", BlueprintReadWrite)
	TWeakObjectPtr<UObject> WorldContext = nullptr;
	
	virtual float GetFloat()
	{
		/**Base struct should not be used. Always return 0*/
		return 0;
	}
};

/**
 * Wrapper struct to achieve 3 things:
 * 1. It does not seem to be possible to customize the
 * header for TInstancedStruct variables, UNLESS you wrap
 * them into another struct. All in-engine examples do this.
 * 2. Since blueprint users cannot create instanced structs,
 * they can make this struct, and we will handle the rest.
 * 3. Since we can now customize the provided FloatProvider,
 * we can enforce a basic float at all times. This means
 * there's no risk of accidentally assigning a null
 * float provider type, which could cause crashes
 * or unwanted behavior.
 */
USTRUCT(BlueprintType)
struct FOmniFloatProvider
{
	GENERATED_BODY()

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite, meta = (ShowTreeView))
	TInstancedStruct<FFloatProviderData> FloatProvider;

	float GetFloat()
	{
		return FloatProvider.GetMutable<>().GetFloat();
	}
};

USTRUCT(BlueprintType, DisplayName = "Basic Float")
struct FBasicFloatProvider : public FFloatProviderData
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
	float FloatValue = 0;

	virtual float GetFloat() override
	{
		return FloatValue;
	}
};

USTRUCT(BlueprintType, DisplayName = "Runtime Curve")
struct FRuntimeFloatProvider : public FFloatProviderData
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve FloatCurve;

	virtual float GetFloat() override
	{
		return FloatCurve.GetRichCurve()->Eval(BaseValue);
	}
};

/**
 * Uncomment if you want a scalable float provider.
 * IMPORTANT: Uncomment the GameplayAbilities in the
 * build.cs's PublicDependencyModuleNames and add
 * #include "ScalableFloat.h" to the top of this file
 * */
// USTRUCT(BlueprintType, DisplayName = "Scalable Float")
// struct FScalableFloatProvider : public FFloatProvider
// {
// 	GENERATED_BODY()
// 	
// 	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
// 	FScalableFloat ScalableFloat;
//
// 	virtual float GetFloat() override
// 	{
// 		return ScalableFloat.GetValueAtLevel(BaseValue);
// 	}
// };

/**Get the value of a tag of the assigned world context.
 * Only works if the ObjectTags plugin is installed and enabled.
 *
 * Requires world context to be assigned. */
USTRUCT(BlueprintType, DisplayName = "Object Tag Value")
struct FObjectTagValueProvider : public FFloatProviderData
{
	GENERATED_BODY()

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
	FGameplayTag Tag;

	virtual float GetFloat() override
	{
		#if ObjectTags_Enabled
		return UObjectTags_Subsystem::GetTagValueFromObject(Tag, WorldContext.Get());
		#else

		return 0;
		#endif
	}
};

/**Data table struct to be used with float providers.
 * Each row contains just a single float, then float providers
 * retrieve a specific row to get the float value. */
USTRUCT(BlueprintType)
struct FFloatProviderDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
	float Value = 0;
};

/**Retrieves a float from a data table and a specific row.*/
USTRUCT(BlueprintType, DisplayName = "Float Table")
struct FDataTableFloatProvider : public FFloatProviderData
{
	GENERATED_BODY()

	UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly, meta = (RowType = "/Script/OmniToolbox.FloatProviderDataTable"))
	FDataTableRowHandle FloatTable;

	virtual float GetFloat() override
	{
		if(FloatTable.DataTable == nullptr)
		{
			return 0;
		}
		
		return FloatTable.DataTable->FindRow<FFloatProviderDataTable>(FloatTable.RowName, "")->Value;
	}
};