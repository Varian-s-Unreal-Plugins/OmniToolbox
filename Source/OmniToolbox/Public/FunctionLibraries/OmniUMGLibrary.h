// Copyright (C) Varian Daemon 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OmniUMGLibrary.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API UOmniUMGLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**Projects a world location onto the screen, continuing to
	 * calculate it while the position is off-screen.
	 *
	 * Credit goes to the Epic forums. I don't remember exactly
	 * which post most of the math for this node came from,
	 * but it is slightly modified from its source.
	 * TODO: Fix the snapping when going from hugging the boundaries of the screen to the WorldLocation*/
	UFUNCTION(Category = "OmniToolbox|UI Library", BlueprintPure, meta = (Keywords = "world location to screen position"))
	static bool ProjectWorldToScreen(APlayerController* PlayerController, FVector WorldLocation, FVector2D& OutScreenPosition, bool& LocationIsInViewport, float& OffScreenAngle);
	
	UFUNCTION(Category = "OmniToolbox|UI Library", BlueprintCallable, BlueprintPure)
	static UWidget* GetFocusedWidget();

	UFUNCTION(Category = "OmniToolbox|UI Library", BlueprintCallable)
	static void SetEnableWorldRendering(bool Enable);
};
