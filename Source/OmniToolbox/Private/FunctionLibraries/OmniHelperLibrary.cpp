// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniHelperLibrary.h"

#include "FloatProvider.h"
#include "Components/ActorComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

#include "Interfaces/IPluginManager.h"

void UOmniHelperLibrary::SetAllowAnyoneToDestroyMe(UActorComponent* Component, bool Allow)
{
	Component->bAllowAnyoneToDestroyMe = Allow;
}

FString UOmniHelperLibrary::GetPropertyByNameAsString(UObject* Object, FName PropertyName)
{
	if (!Object)
	{
		return FString(TEXT("Invalid Object"));
	}

	FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
	if (!Property)
	{
		return FString(TEXT("Property Not Found"));
	}

	void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);

	FString PropertyValueString;
	Property->ExportTextItem_Direct(PropertyValueString, PropertyValue, PropertyValue, nullptr, PPF_None);

	return PropertyValueString;
}

void UOmniHelperLibrary::SetObjectOwner(UObject* Object, UObject* NewOwner)
{
	if(!Object || !NewOwner)
	{
		return;
	}

	if(Object->GetOuter() == NewOwner)
	{
		return;
	}

	Object->Rename(nullptr, NewOwner);
}

TArray<FString> UOmniHelperLibrary::GetAllGameFeatureNames()
{
	TArray<FString> PluginNames;
	//Get all plugins (both enabled and disabled)
	TArray<TSharedRef<IPlugin>> AllPlugins = IPluginManager::Get().GetDiscoveredPlugins();
	for(const TSharedRef<IPlugin>& Plugin : AllPlugins)
	{
		//Check if the plugin is marked as a modular game feature plugin
		if(Plugin->GetDescriptor().Category == "Game Features")
		{
			PluginNames.Add(Plugin->GetName());
		}
	}
	return PluginNames;
}

TArray<UObject*> UOmniHelperLibrary::GetObjectsSubObjects(UObject* Object, bool Recursive)
{
	TArray<UObject*> SubObjects;
	GetObjectsWithOuter(Object, SubObjects, Recursive);
	return SubObjects;
}

FInstancedStruct UOmniHelperLibrary::GetStructFromInstancedStructArray(TArray<FInstancedStruct> Array,
	const UScriptStruct* Struct)
{
	for(auto& CurrentStruct : Array)
	{
		if(CurrentStruct.GetScriptStruct() == Struct)
		{
			return CurrentStruct;
		}
	}
	
	return FInstancedStruct();
}

TArray<FInstancedStruct> UOmniHelperLibrary::GetStructArrayFromInstancedStructArray(TArray<FInstancedStruct> Array,
	const UScriptStruct* Struct)
{
	TArray<FInstancedStruct> OutArray;
	
	for(auto& CurrentStruct : Array)
	{
		if(CurrentStruct.GetScriptStruct() == Struct)
		{
			OutArray.Add(CurrentStruct);
		}
	}
	
	return OutArray;
}

FVector UOmniHelperLibrary::GetCursorPointOnPlane(UObject* WorldContext, const FVector& PlanePoint, const FVector& PlaneNormal)
{
	if(!WorldContext)
	{
		return FVector::ZeroVector;
	}

	APlayerController* PlayerController = WorldContext->GetWorld()->GetFirstPlayerController();
	if(!PlayerController)
	{
		return FVector::ZeroVector;
	}

	float MouseX, MouseY;
	if(!PlayerController || !PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return PlanePoint;
	}

	FVector RayOrigin, RayDir;
	PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, RayOrigin, RayDir);

	const float Denom = FVector::DotProduct(RayDir, PlaneNormal);
	if(FMath::Abs(Denom) < KINDA_SMALL_NUMBER)
	{
		//Ray nearly parallel to plane -> pick a far point on the ray and project that onto the plane.
		FVector Far = RayOrigin + RayDir * 10000.0f;
		float DistToPlane = FVector::DotProduct(Far - PlanePoint, PlaneNormal);
		return Far - PlaneNormal * DistToPlane;
	}

	float T = FVector::DotProduct(PlanePoint - RayOrigin, PlaneNormal) / Denom;
	return RayOrigin + RayDir * T;
}

float UOmniHelperLibrary::GetFloatFromFloatProvider(FOmniFloatProvider FloatProvider)
{
	if(FloatProvider.FloatProvider.IsValid())
	{
		return FloatProvider.FloatProvider.GetMutablePtr<>()->GetFloat();
	}
	else
	{
		return 0;
	}
}

float UOmniHelperLibrary::GetFloatFromInstancedFloatProvider(FInstancedStruct FloatProvider)
{
	if(FloatProvider.IsValid() && FloatProvider.GetMutablePtr<FFloatProviderData>())
	{
		return FloatProvider.GetMutablePtr<FFloatProviderData>()->GetFloat();
	}
	else
	{
		return 0;
	}
}

UOmniDelayWithPayload* UOmniDelayWithPayload::DelayWithPayload(UObject* WorldContext, float Delay,
                                                               FInstancedStruct Payload)
{
	UOmniDelayWithPayload* AsyncNode = NewObject<UOmniDelayWithPayload>(WorldContext);
	AsyncNode->DelayLength = Delay;
	AsyncNode->PayloadData = Payload;
	return AsyncNode;
}

void UOmniDelayWithPayload::Activate()
{	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]
	{
		Completed.Broadcast(PayloadData);
		SetReadyToDestroy();
	}), DelayLength, false);
}

UOmniAsyncLoadClassWithPayload* UOmniAsyncLoadClassWithPayload::AsyncLoadClassWithPayload(UObject* WorldContext,
	TSoftClassPtr<UObject> Class, FInstancedStruct Payload)
{
	UOmniAsyncLoadClassWithPayload* AsyncNode = NewObject<UOmniAsyncLoadClassWithPayload>(WorldContext);
	AsyncNode->ClassToLoad = Class;
	AsyncNode->PayloadData = Payload;
	return AsyncNode;
}

void UOmniAsyncLoadClassWithPayload::Activate()
{
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ClassToLoad.ToSoftObjectPath(), [this]
	{
		Completed.Broadcast(PayloadData);
		SetReadyToDestroy();
	});
}

UOmniAsyncLoadAssetWithPayload* UOmniAsyncLoadAssetWithPayload::AsyncLoadAssetWithPayload(UObject* WorldContext,
	TSoftObjectPtr<UObject> Asset, FInstancedStruct Payload)
{
	UOmniAsyncLoadAssetWithPayload* AsyncNode = NewObject<UOmniAsyncLoadAssetWithPayload>(WorldContext);
	AsyncNode->AssetToLoad = Asset;
	AsyncNode->PayloadData = Payload;
	return AsyncNode;
}

void UOmniAsyncLoadAssetWithPayload::Activate()
{
	UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetToLoad.ToSoftObjectPath(), [this]
	{
		Completed.Broadcast(PayloadData);
		SetReadyToDestroy();
	});
}
