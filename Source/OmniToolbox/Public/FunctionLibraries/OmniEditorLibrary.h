// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
};
