// Copyright (C) Varian Daemon 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "OmniHelperLibrary.generated.h"

/**
 * Find the first struct of type T in an array of FInstancedStruct and return a mutable pointer.
 * Returns nullptr if not found.
 */
template <typename T>
FORCEINLINE T* GetStructPtrFromInstancedStructArray(TArray<FInstancedStruct>& Array)
{
	static_assert(TIsUECoreType<T>::Value == false, "T must be a USTRUCT type.");
	for (FInstancedStruct& Item : Array)
	{
		if (Item.IsValid() && Item.GetScriptStruct() == T::StaticStruct())
		{
			// Safe because we validated the type above
			return Item.GetMutable<T>();
		}
	}
	return nullptr;
}

/**
 * Find the first instance of a struct of type TDesiredStruct inside an array of TInstancedStruct of type TInstancedStructType.
 * Returns nullptr if not found.
 *
 * Example:
 *
 * TArray<TInstancedStruct<FAgentGameplayEffectData> Data;
 * FEffectDataUIIcon* EffectIcon = GetStructPtrFromInstancedStructArray<FEffectDataUIIcon, FAgentGameplayEffectData>(Data)
 */
template<typename TDesiredStruct, typename TInstancedStructType>
FORCEINLINE TDesiredStruct* GetStructPtrFromInstancedStructArray(TArray<TInstancedStruct<TInstancedStructType>>& Array)
{
	static_assert(TIsUECoreType<TDesiredStruct>::Value == false, "T must be a USTRUCT type.");
	for (TInstancedStruct<TInstancedStructType>& Item : Array)
	{
		if (Item.IsValid() && Item.GetScriptStruct() == TDesiredStruct::StaticStruct())
		{
			// Safe because we validated the type above
			return Item.template GetMutablePtr<TDesiredStruct>();
		}
	}
	return nullptr;
}

/**
 * Const overload: find the first struct of type T and return a const pointer.
 * Returns nullptr if not found.
 */
template <typename T>
FORCEINLINE const T* GetStructPtrFromInstancedStructArray(const TArray<FInstancedStruct>& Array)
{
	static_assert(TIsUECoreType<T>::Value == false, "T must be a USTRUCT type.");
	for (const FInstancedStruct& Item : Array)
	{
		if (Item.IsValid() && Item.GetScriptStruct() == T::StaticStruct())
		{
			// Safe because we validated the type above
			return Item.Get<T>();
		}
	}
	return nullptr;
}

/**
 * Generic and common helper functions that might not fit in any other category.
 */
UCLASS()
class OMNITOOLBOX_API UOmniHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**Epic in all their glory have a hidden parameter that you can't access in any way
	 * in Blueprints and thus can't call DestroyComponent on a component without calling it in
	 * the actor BP that owns the component because they hid the Object pin, just
	 * why Epic just allow me to set this boolean in blueprints.*/
	UFUNCTION(Category = "OmniToolbox", BlueprintCallable)
	static void SetAllowAnyoneToDestroyMe(UActorComponent* Component, bool Allow);
	
	UFUNCTION(Category = "OmniToolbox", BlueprintCallable)
	static FString GetPropertyByNameAsString(UObject* Object, FName PropertyName);

	/**Forcefully rename an object and assign a new outer.
	 * Only use this if you must.
	 * Most of the time, this is used to change owners of an
	 * actor component.*/
	UFUNCTION(Category = "OmniToolbox", BlueprintCallable)
	static void SetObjectOwner(UObject* Object, UObject* NewOwner);
	
	UFUNCTION(Category = "OmniToolbox|Modular Game Features", BlueprintCallable)
	static TArray<FString> GetAllGameFeatureNames();

	UFUNCTION(Category = "OmniToolbox", BlueprintCallable)
	static TArray<UObject*> GetObjectsSubObjects(UObject* Object, bool Recursive = true);

	UFUNCTION(Category = "OmniToolbox", BlueprintPure)
	static FInstancedStruct GetStructFromInstancedStructArray(TArray<FInstancedStruct> Array, const UScriptStruct* Struct);
	
	UFUNCTION(Category = "OmniToolbox", BlueprintPure)
	static TArray<FInstancedStruct> GetStructArrayFromInstancedStructArray(TArray<FInstancedStruct> Array, const UScriptStruct* Struct);
	
	UFUNCTION(Category = "OmniToolbox|Cursor", BlueprintCallable, meta = (WorldContext = "WorldContext", DefaultToSelf = "WorldContext"))
	static FVector GetCursorPointOnPlane(UObject* WorldContext, const FVector& PlanePoint, const FVector& PlaneNormal);
};
