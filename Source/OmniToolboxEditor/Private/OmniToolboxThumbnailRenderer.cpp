// Copyright (C) Varian Daemon. All Rights Reserved


#include "OmniToolboxThumbnailRenderer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "OmniRuntimeMacros.h"
#include "Config/DS_OmniToolboxSettings.h"
#include "Developer/I_AssetDetails.h"

Omni_OnModuleStarted("OmniToolbox")
{
	//Disable the old renderers and enable our custom renderers
	UThumbnailManager::Get().UnregisterCustomRenderer(UBlueprint::StaticClass());
	UThumbnailManager::Get().RegisterCustomRenderer(UBlueprint::StaticClass(), UOmniToolboxThumbnailRenderer::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UDataAsset::StaticClass());
	UThumbnailManager::Get().RegisterCustomRenderer(UDataAsset::StaticClass(), UOmniToolboxThumbnailRenderer::StaticClass());
}

void UOmniToolboxThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height,
                                         FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	UDataAsset* DataAsset = Cast<UDataAsset>(Object);
	if(!Blueprint && !DataAsset)
	{
		Super::Draw(Object, X, Y, Width, Height, Viewport, Canvas, bAdditionalViewFamily);
		return;
	}

	//Background by default is pitch black. Set it to the normal gray background.
	//V: There must be a better way of getting the values.
	//V: I think these values are getting rounded somewhere? The color is sometimes *slightly* different
	Canvas->Clear(FLinearColor(FColor(90, 90, 90)));
	
	UTexture2D* ThumbnailToUse = FindThumbnailOverrideForObject(Object);
	
	if(ThumbnailToUse)
	{
		/**Not all textures are a 1:1 aspect ratio. If we just used Width|Height to render the final canvas,
		 * it would stretch. This ensures the aspect ratio is correct.*/
		float MaxRange = Width > Height ? Width : Height;
		float MaxImageRange = ThumbnailToUse->GetResource()->GetSizeX() > ThumbnailToUse->GetResource()->GetSizeY() ?
			ThumbnailToUse->GetResource()->GetSizeX() : ThumbnailToUse->GetResource()->GetSizeY();
		float NormalizeWidth = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxImageRange),
			FVector2D(0, MaxRange), ThumbnailToUse->GetResource()->GetSizeX());
		float NormalizedHeight = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxImageRange),
	FVector2D(0, MaxRange), ThumbnailToUse->GetResource()->GetSizeY());
		//Remember to offset the Y position to make sure textures that aren't 1:1 get centered
		FCanvasTileItem CanvasTile(FVector2D((Width - NormalizeWidth) / 2, (Height - NormalizedHeight) / 2),
			ThumbnailToUse->GetResource(), FVector2D(NormalizeWidth, NormalizedHeight), FLinearColor::White);
		CanvasTile.BlendMode = SE_BLEND_Translucent;
		CanvasTile.Draw(Canvas);
		return;
	}
	
	Super::Draw(Object, X, Y, Width, Height, Viewport, Canvas, bAdditionalViewFamily);
}

bool UOmniToolboxThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if(Super::CanVisualizeAsset(Object))
	{
		return true;
	}

	UTexture2D* ThumbnailToUse = FindThumbnailOverrideForObject(Object);
	if(ThumbnailToUse)
	{
		return true;
	}

	return false;
}

UTexture2D* UOmniToolboxThumbnailRenderer::FindThumbnailOverrideForObject(UObject* Object)
{
	if(!Object)
	{
		return nullptr;
	}

	//Blueprint objects true class can only be fetched through GeneratedClass.Get()
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	
	UClass* ObjectClass = Blueprint ? Blueprint->GeneratedClass.Get() : Object->GetClass();
	if(!ObjectClass)
	{
		return nullptr;
	}
	
	//ThumbnailOverrides has the highest priority, if anything is found, use that texture.
	const UDS_OmniToolboxSettings* Settings = GetDefault<UDS_OmniToolboxSettings>();
	for(auto& CurrentThumbnail : Settings->ThumbnailOverrides)
	{
		if(CurrentThumbnail.Key.ToSoftObjectPath().IsValid() && CurrentThumbnail.Value.ToSoftObjectPath().IsValid())
		{
			//Find out if the class is in the override map or is a child of the classes in the map
			if(ObjectClass == CurrentThumbnail.Key.LoadSynchronous() || ObjectClass->IsChildOf(CurrentThumbnail.Key.LoadSynchronous()))
			{
				return CurrentThumbnail.Value.LoadSynchronous();;
			}
		}
	}

	if(Object->Implements<UI_AssetDetails>())
	{
		if(UTexture2D* Texture = II_AssetDetails::Execute_GetThumbnail(Object))
		{
			return Texture;
		}
	}

	//Go through all hard object references and find any with the name "ThumbnailOverride". If found, use that.
	for(const auto& [Property, Value] : TPropertyValueRange<FObjectProperty>(ObjectClass, ObjectClass->GetDefaultObject()))
	{
		if(Property->PropertyClass != UTexture2D::StaticClass() || !Property->GetName().Equals("ThumbnailOverride"))
		{
			continue;
		}

		void* MutableValue = const_cast<void*>(Value);
		UTexture2D* Texture = *static_cast<UTexture2D**>(MutableValue);
		if(!Texture)
		{
			continue;
		}
		
		return Texture;
	}

	//Go through all soft object references and find any with the name "ThumbnailOverride". If found, use that.
	for(const auto& [Property, Value] : TPropertyValueRange<FSoftObjectProperty>(ObjectClass, ObjectClass->GetDefaultObject()))
	{
		if(Property->PropertyClass != UTexture2D::StaticClass() || !Property->GetName().Equals("ThumbnailOverride"))
		{
			continue;
		}

		void* MutableValue = const_cast<void*>(Value);
		const TSoftObjectPtr<UTexture2D>& SoftTexture = *static_cast<TSoftObjectPtr<UTexture2D>*>(MutableValue);
		if(SoftTexture.ToSoftObjectPath().IsValid())
		{
			return SoftTexture.LoadSynchronous();
		}
	}

	//Go through any soft object properties with the ThumbnailOverride metadata. If found, use that.
	//NOTE: This is repeated again after this, but on a different class. I don't know why, but if a class
	//has the property defined in C++, then a blueprint is made out of it, the thumbnail fails to be found.
	//Repeating it again on the ObjectClass fixes it.
	for(const auto& [Property, Value] : TPropertyValueRange<FSoftObjectProperty>(ObjectClass, ObjectClass->GetDefaultObject()))
	{
		if(Property->PropertyClass != UTexture2D::StaticClass() || !Property->HasMetaData("ThumbnailOverride"))
		{
			continue;
		}
	
		void* MutableValue = const_cast<void*>(Value);
		const TSoftObjectPtr<UTexture2D>& SoftTexture = *static_cast<TSoftObjectPtr<UTexture2D>*>(MutableValue);
		if(SoftTexture.ToSoftObjectPath().IsValid())
		{
			return SoftTexture.LoadSynchronous();
		}
	}

	//Go through any soft object properties with the ThumbnailOverride metadata. If found, use that.
	for(const auto& [Property, Value] : TPropertyValueRange<FSoftObjectProperty>(Object->GetClass(), Object))
	{
		if(Property->PropertyClass != UTexture2D::StaticClass() || !Property->HasMetaData("ThumbnailOverride"))
		{
			continue;
		}

		void* MutableValue = const_cast<void*>(Value);
		const TSoftObjectPtr<UTexture2D>& SoftTexture = *static_cast<TSoftObjectPtr<UTexture2D>*>(MutableValue);
		if(SoftTexture.ToSoftObjectPath().IsValid())
		{
			return SoftTexture.LoadSynchronous();
		}
	}

	//Go through any hard object properties with the ThumbnailOverride metadata. If found, use that.
	for(const auto& [Property, Value] : TPropertyValueRange<FObjectProperty>(Object->GetClass(), Object))
	{
		if(Property->PropertyClass != UTexture2D::StaticClass() || !Property->HasMetaData("ThumbnailOverride"))
		{
			continue;
		}

		void* MutableValue = const_cast<void*>(Value);

		UTexture2D* Texture = *static_cast<UTexture2D**>(MutableValue);
		if(!Texture)
		{
			continue;
		}
		
		return Texture;
	}

	return nullptr;
}
