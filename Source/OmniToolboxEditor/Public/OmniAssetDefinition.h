// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"

#include "OmniAssetDefinition.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOXEDITOR_API UOmniAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:

	TSoftClassPtr<UObject> Class;
	FText AssetDisplayName;
	FLinearColor AssetColor;
	FAssetCategoryPath Categories = FAssetCategoryPath(FText::FromString(""));;

	virtual TSoftClassPtr<UObject> GetAssetClass() const override
	{
		return Class;
	}

	virtual FText GetAssetDisplayName() const override
	{
		return AssetDisplayName;
	}

	virtual FLinearColor GetAssetColor() const override
	{
		return AssetColor;
	}

	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		return TConstArrayView<FAssetCategoryPath>(&Categories, 1);
	}
};

UCLASS()
class UOmniFactory : public UFactory
{
	GENERATED_BODY()

public:

	UOmniFactory()
	{
		bCreateNew = true;	
	};

	virtual void PostInitProperties() override;

	bool UseChildrenDialogueWindow = false;

	virtual bool CanCreateNew() const override
	{
		return IsValid(SupportedClass);
	}

	virtual bool ShouldShowInNewMenu() const override;
	virtual FText GetDisplayName() const override;

	virtual bool ConfigureProperties() override;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	virtual FString GetToolTipDocumentationPage() const override;
	virtual FString GetToolTipDocumentationExcerpt() const override;
};

/**V: This solution is absolutely cursed, but it's the only way I've been able to find to work
 * around the janky content browser factory code in the engine.
 *
 * Problem 1: Each asset needs its own factory.
 * Problem 2: Factories cannot support multiple classes
 * Problem 3: Some UE code uses the class default object, meaning NewObject won't work
 * since UE will just fetch the CDO, which all assets would share.
 *
 * Since UE expects factories to only support 1 class, we can't make a factory that
 * supports multiple classes This means each asset must have a UNIQUE factory
 *
 * I analyzed VoxelCore's method and I have to agree with their solution of using
 * "dummy" factories to give each asset a unique factory.
 *
 * This technically means there's a hard cap on how many assets can be added, but it's
 * straightforward to add more. */

UCLASS() class UOmniFactoryDummy0  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy1  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy2  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy3  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy4  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy5  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy6  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy7  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy8  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy9  : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy10 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy11 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy12 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy13 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy14 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy15 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy16 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy17 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy18 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy19 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy20 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy21 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy22 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy23 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy24 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy25 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy26 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy27 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy28 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy29 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy30 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy31 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy32 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy33 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy34 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy35 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy36 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy37 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy38 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy39 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy40 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy41 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy42 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy43 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy44 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy45 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy46 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy47 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy48 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy49 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy50 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy51 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy52 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy53 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy54 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy55 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy56 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy57 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy58 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy59 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy60 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy61 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy62 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy63 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy64 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy65 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy66 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy67 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy68 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy69 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy70 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy71 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy72 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy73 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy74 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy75 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy76 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy77 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy78 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy79 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy80 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy81 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy82 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy83 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy84 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy85 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy86 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy87 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy88 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy89 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy90 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy91 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy92 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy93 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy94 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy95 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy96 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy97 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy98 : public UOmniFactory { GENERATED_BODY() };
UCLASS() class UOmniFactoryDummy99 : public UOmniFactory { GENERATED_BODY() };