// Copyright (C) Varian Daemon. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/BlueprintThumbnailRenderer.h"
#include "OmniToolboxThumbnailRenderer.generated.h"

/**
 * 
 */
UCLASS()
class OMNITOOLBOXEDITOR_API UOmniToolboxThumbnailRenderer : public UBlueprintThumbnailRenderer
{
	GENERATED_BODY()

	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;

	virtual bool CanVisualizeAsset(UObject* Object) override;

	TClassInstanceThumbnailScene<FBlueprintThumbnailScene, 100> ThumbnailScenes;

public:

	/**Go through all scenarios and find a thumbnail override for the object.
	 * If @ClearThumbnail is true, it means we found a thumbnail override,
	 * but the texture had not been set.*/
	static UTexture2D* FindThumbnailOverrideForObject(UObject* Object);
};
