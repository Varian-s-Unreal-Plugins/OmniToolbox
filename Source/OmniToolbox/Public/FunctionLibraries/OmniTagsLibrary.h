// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OmniTagsLibrary.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API UOmniTagsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(Category = "OmniToolbox|Tags Library", BlueprintCallable, BlueprintPure)
	static FGameplayTagContainer GetTagsChildren(FGameplayTag Tag, bool Recursive);

	UFUNCTION(Category = "OmniToolbox|Tags Library", BlueprintCallable, BlueprintPure)
	static bool DoesTagHaveChildren(FGameplayTag Tag);

	/**Find out if @Tag is a child of @PotentialParent.
	 * If @DirecChild is true, then @Tag must be a direct child of @PotentialTag
	 * For example; returns true if @Tag is Report.CauseOfDeath.Stabbed and @PotentialParent
	 * is Report.CauseOfDeath.*/
	UFUNCTION(Category = "OmniToolbox|Tags Library", BlueprintCallable, BlueprintPure)
	static bool IsTagChildOfTag(FGameplayTag Tag, FGameplayTag PotentialParent, bool DirectChild);

	/** Returns direct parent GameplayTag of this GameplayTag, calling on x.y will return x */
	UFUNCTION(Category = "OmniToolbox|Tags Library", BlueprintCallable, BlueprintPure)
	static FGameplayTag GetParentTag(FGameplayTag Tag);

	/**Adds a new tag to an ini file.
	 * @TagName The final name of the tag
	 * @TagComment The comment to give the tag
	 * @IniName The name of the ini file we wish to modify.
	 * This field should include .ini at the end */
	UFUNCTION(Category = "OmniToolbox|Tags Library|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static bool AddNewTagToINI(FString TagName, FString TagComment, FName IniName);
};
