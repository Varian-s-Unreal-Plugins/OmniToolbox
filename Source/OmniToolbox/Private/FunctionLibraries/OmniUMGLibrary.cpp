// Copyright (C) Varian Daemon 2023. All Rights Reserved.


#include "FunctionLibraries/OmniUMGLibrary.h"

#include "Engine/GameViewportClient.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/UObjectIterator.h"

bool UOmniUMGLibrary::ProjectWorldToScreen(APlayerController* PlayerController, FVector WorldLocation,
                                           FVector2D& OutScreenPosition, bool& LocationIsInViewport, float& OffScreenAngle)
{
	if(!PlayerController)
	{
		return false;
	}
	
	OffScreenAngle = 0.0f;

	FVector2D ScreenPosTemp(-1.f, -1.f);

	// UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, WorldLocation, ScreenPosTemp, true);
	PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenPosTemp, true);

	const FVector2D ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
	LocationIsInViewport = ScreenPosTemp.X >= 0.0f && ScreenPosTemp.X <= ViewportSize.X && ScreenPosTemp.Y >= 0.0f && ScreenPosTemp.Y <= ViewportSize.Y;

	if (LocationIsInViewport) /*It is on the screen*/
	{
		/**V: The normal math for this function is slightly off, but I do not know enough
		 * about projecting 3D coordinates onto a 2D screen to know where to try and fix
		 * this. The issue can be better seen by inversing the commenting and non-commented
		 * code in this block (except for "return true;").*/
		
		// OutScreenPosition = ScreenPosTemp;
		FVector2D ScreenPosition(FMath::RoundToInt(ScreenPosTemp.X), FMath::RoundToInt(ScreenPosTemp.Y));

		FVector2D ViewportPosition2D;
		USlateBlueprintLibrary::ScreenToViewport(PlayerController, ScreenPosition, ViewportPosition2D);
		OutScreenPosition.X = ViewportPosition2D.X;
		OutScreenPosition.Y = ViewportPosition2D.Y;
		return true;
	}
	else /*It is not on the screen*/
	{
		const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
		const FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

		const FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
		const FVector CameraRightVector = UKismetMathLibrary::GetRightVector(CameraRotation);
		const FVector CameraUpVector = UKismetMathLibrary::GetUpVector(CameraRotation);

		const FVector ProjectedVector = UKismetMathLibrary::ProjectVectorOnToPlane(WorldLocation - CameraLocation, CameraForwardVector).GetSafeNormal();

		// Some random calculation to determine wheter the angle is in upper or lower half
		const float DotProduct = FVector::DotProduct(ProjectedVector, CameraUpVector);

		// Set angle to -180 .. 180 range
		if (DotProduct > 0.0f)
		{
			const float Dot = FVector::DotProduct(ProjectedVector, CameraRightVector);
			OffScreenAngle = -UKismetMathLibrary::DegAcos(Dot); // negative angle
		}
		else
		{
			const float Dot = FVector::DotProduct(ProjectedVector, CameraRightVector);
			OffScreenAngle = UKismetMathLibrary::DegAcos(Dot); // positive angle
		}

		// Coordinates of 4 borders in viewport screen
		const FVector UpperLeftCorner(0.f, 0.f, 0.f);
		const FVector LowerLeftCorner(0.f, ViewportSize.Y, 0.f);
		const FVector UpperRightCorner(ViewportSize.X, 0.f, 0.f);
		const FVector LowerRightCorner(ViewportSize.X, ViewportSize.Y, 0.f);		
		
		const FVector ViewportCentre(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f, 0.f);		

		// Vector that points from centre of the screen to the WorldPosition
		FVector DirectionVector(UKismetMathLibrary::DegCos(OffScreenAngle), UKismetMathLibrary::DegSin(OffScreenAngle), 0.f);
		DirectionVector *= 10'000.f;	// multiplying by some value to make sure end of this vector will always lie outside of the screen
		DirectionVector += ViewportCentre;

		FVector IntersectionPoint = FVector::ZeroVector;

		if (OffScreenAngle <= 90.f)
		{
			if (OffScreenAngle <= 0.f)
			{
				if (OffScreenAngle <= -90.f)
				{
					// 2. Quadrant						
					FMath::SegmentIntersection2D(UpperLeftCorner, UpperRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);
					FMath::SegmentIntersection2D(UpperLeftCorner, LowerLeftCorner, ViewportCentre, DirectionVector, IntersectionPoint);
				}
				else
				{
					// 1. Quadrant						
					FMath::SegmentIntersection2D(UpperLeftCorner, UpperRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);
					FMath::SegmentIntersection2D(UpperRightCorner, LowerRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);
				}
			}
			else
			{
				// 4. Quadrant				
				FMath::SegmentIntersection2D(LowerLeftCorner, LowerRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);
				FMath::SegmentIntersection2D(UpperRightCorner, LowerRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);
			}
		}
		else
		{
			// 3. Quadrant			
			FMath::SegmentIntersection2D(UpperLeftCorner, LowerLeftCorner, ViewportCentre, DirectionVector, IntersectionPoint);
			FMath::SegmentIntersection2D(LowerLeftCorner, LowerRightCorner, ViewportCentre, DirectionVector, IntersectionPoint);

		}

		OutScreenPosition.X = IntersectionPoint.X;
		OutScreenPosition.Y = IntersectionPoint.Y;

		return false;
	}	
}

UWidget* UOmniUMGLibrary::GetFocusedWidget()
{
	TSharedPtr<SWidget> FocusedSlateWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
	if (!FocusedSlateWidget.IsValid())
	{
		return nullptr;
	}
	for (TObjectIterator<UWidget> Itr; Itr; ++Itr)
	{
		UWidget* CandidateUMGWidget = *Itr;
		if (CandidateUMGWidget->GetCachedWidget() == FocusedSlateWidget)
		{
			return CandidateUMGWidget;
		}
	}
	return nullptr;
}

void UOmniUMGLibrary::SetEnableWorldRendering(bool Enable)
{
	if(UGameViewportClient* ViewportClient = GEngine->GameViewport)
	{
		ViewportClient->bDisableWorldRendering = Enable;
	}
}
