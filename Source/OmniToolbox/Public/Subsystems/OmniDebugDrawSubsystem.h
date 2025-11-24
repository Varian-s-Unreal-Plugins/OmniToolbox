// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OmniDebugDrawSubsystem.generated.h"

UENUM()
enum EOmniDebugDrawType
{
	Circle,
	Line,
	Box,
	Sphere,
	Capsule,
	Arrow,
	Text,
	Cone
};

/**In an effort to reduce development time for a debugging tool,
 * I've decided to make one monolithic struct for all debug types.
 * It's not pretty, but it's quick to implement. */
USTRUCT()
struct FOmniDebugDrawCommand
{
	GENERATED_BODY()
	
	EOmniDebugDrawType Type;
	
	FVector Location;
	
	FVector End;
	
	FVector Direction;
	
	FVector Extent;
	
	float Length;
	
	float Radius;
	
	float AngleWidth;
	float AngleHeight;
	
	float HalfHeight;
	
	FQuat Rotation;
	
	FLinearColor Color;
	
	uint8 DepthPriority;
	
	float Thickness;
	
	float ArrowSize;
	
	FName LogCategory;
	
	FString Text;
	
	bool AddMessageToLog = false;
	
	bool Wireframe = false;
	
	TWeakObjectPtr<UObject> Owner;
	
	float Lifetime = 3;
};

/**
 * This subsystem is responsible for managing and drawing a list
 * of debug shapes and allowing them to be updated
 */
UCLASS()
class OMNITOOLBOX_API UOmniDebugDrawSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	void AddShape(FOmniDebugDrawCommand Command, FName Key);
	
	TMap<FName, FOmniDebugDrawCommand> ShapesToDraw;

	virtual TStatId GetStatId() const override
	{
		return TStatId();
	}
	
	virtual void Tick(float DeltaTime) override;
	
	
private:
	static constexpr ELogVerbosity::Type DefaultVerbosity = ELogVerbosity::Log;
};
