// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OmniEditorLibrary.generated.h"

#define Omni_Notification(Message) UOmniEditorLibrary::SendNotification(#Message);

/**
 * Library for providing useful in-editor functions.
 * This is NOT in an editor module to simplify the API.
 *
 * You should assume that most functions in this library
 * do not work in-game and especially packaged builds,
 * similar to print string, draw debugs, etc.
 */
UCLASS()
class OMNITOOLBOX_API UOmniEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static bool ImplementInterface(UObject* InstigatingObject, UObject* TargetObject, UClass* Interface, TArray<FString> CustomMessages = TArray<FString>{});

	/**Gives an object a new display name. This is useful for debugging, since it'll
	 * give the object a distinct name in the debug object selection dropdown menu */
	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void SetObjectDisplayName(UObject* Object, const FString& NewName);

	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void SendNotificationWithState(FString Message, bool Success);

	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void SendNotification(FString Message);

	/**Since Epic in all their glory decided to make UKismetSystemLibrary::RaiseScriptError
	 * not exportable, we have made our own.
	 * If Epic ever add the ENGINE_API macro, this will be deprecated and changed to
	 * simply call the regular RaiseScriptError */
	static void RaiseScriptError(const FString& Message);
	
	/**When adding components to a level actor through an editor tool,
	 * it might in some cases not serialize or appear correctly.
	 * This calls a C++-only function, which is meant to handle that scenario. */
	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void AddInstanceComponent(AActor* Actor, UActorComponent* Component);
	
	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void SetComponentEditorOnly(UActorComponent* Component, bool NewEditorOnly);
	
	UFUNCTION(Category = "OmniToolbox|Editor", BlueprintCallable, meta = (DevelopmentOnly))
	static void MakeComponentInvisibleInActorComponents(UActorComponent* Component);
	
	static FString GetPluginNameForClass(UClass* Class);
	
#pragma region DrawingAndVislog
	
	UFUNCTION(BlueprintCallable, Category = "OmniToolbox|Editor", meta = (CallableWithoutWorldContext, DevelopmentOnly))
	static void EnableVislogRecordingToFile(bool bEnabled);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=7))
	static void DrawAndLogCapsule(UObject* WorldContextObject, FVector Center, float HalfHeight, float Radius, FQuat Rotation, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, 
		FName LogCategory = TEXT("VisLog"), float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogLine(UObject* WorldContextObject, FVector Start, FVector End, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"),
		float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogCone(UObject* WorldContextObject, FVector Start, FVector Direction, float Length, float Angle, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"),
	                           float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogCircle(UObject* WorldContextObject, FVector Center, FVector UpAxis, float Radius, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"), 
		float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogBox(UObject* WorldContextObject, FVector Center, FVector Extent, FString Text, FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"), 
		FName Key = "None", float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogSphere(UObject* WorldContextObject, FVector Center, float Radius, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"), 
		float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogArrow(UObject* WorldContextObject, FVector Start, FVector End, float ArrowSize, FString Key = "", FString Text = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"), 
		float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float Thickness = 1.5);
	
	UFUNCTION(Category = "OmniToolbox|Draw Debug", BlueprintCallable, meta = (DevelopmentOnly, WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay=6))
	static void DrawAndLogText(UObject* WorldContextObject, FVector Location, FString Text, FString Key = "", FLinearColor Color = FLinearColor::White, FName LogCategory = TEXT("VisLog"), 
		float Lifetime = 3, bool bAddToMessageLog = false, bool bWireframe = true, EDrawDebugSceneDepthPriorityGroup DepthPriority = EDrawDebugSceneDepthPriorityGroup::World, float FontSize = 1);
	
#pragma endregion
};
