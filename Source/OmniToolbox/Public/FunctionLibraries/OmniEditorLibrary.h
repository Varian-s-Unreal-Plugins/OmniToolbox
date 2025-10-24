// Copyright (C) Varian Daemon 2023. All Rights Reserved.

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
};
