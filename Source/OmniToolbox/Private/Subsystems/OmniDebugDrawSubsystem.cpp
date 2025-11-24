// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Subsystems/OmniDebugDrawSubsystem.h"

#include "OmniRuntimeMacros.h"
#include "VisualLogger/VisualLoggerKismetLibrary.h"

Omni_ConsoleVariable(
	OMNITOOLBOX_API, bool, DrawDebugShapes, 1,
	"OmniToolbox.Debug.DrawDebugShapes",
	"Allow the OmniToolbox debug draw subsystem to draw debug shapes");

#define VLOG_BP_LIBRARY_ADD_TO_LOG(CategoryName, Format, ...) \
if (IsInGameThread())\
{\
FMessageLog(CategoryName).Info(FText::FromString(FString::Printf((Format), __VA_ARGS__)));\
}\
else\
{\
FMsg::Logf(__FILE__, __LINE__, UE_FNAME_TO_LOG_CATEGORY_NAME(CategoryName), DefaultVerbosity, (Format), __VA_ARGS__);\
}

void UOmniDebugDrawSubsystem::AddShape(FOmniDebugDrawCommand Command, FName Key)
{
	if(Key.IsNone())
	{
		FGuid RandomGuid = FGuid::NewGuid();
		Key = FName(*RandomGuid.ToString(EGuidFormats::Digits));
	}
	
	ShapesToDraw.Add(Key, Command);
}

void UOmniDebugDrawSubsystem::Tick(float DeltaTime)
{
	TArray<FName> ShapesToRemove;
	
	/**Start going over every command and draw them.
	 * V: Small note; you're not "really supposed to" use most kismet libraries
	 * in C++. But in this case, the kismet library does exactly what we
	 * want to do. They handle the preprocessor macro for us and they handle
	 * the message log for us, and we want as much parity between the blueprint
	 * nodes as possible. */
	for(auto& CurrentCommand : ShapesToDraw)
	{
		FOmniDebugDrawCommand& Command = CurrentCommand.Value;

		switch(Command.Type) {
		case Circle:
			{
				if(DrawDebugShapes)
				{
					DrawDebugCircle(GetWorld(), Command.Location, Command.Radius, 16, Command.Color.ToFColor(true),
						false, Command.Lifetime, Command.DepthPriority, Command.Thickness, Command.Rotation.GetAxisY(), Command.Rotation.GetAxisZ(), false);
				}
				#if ENABLE_VISUAL_LOG
				FVisualLogger::DiscLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, Command.Rotation.GetForwardVector(), Command.Radius, 
					Command.Color.ToFColor(true), Command.Thickness /**For some reason the thickness for discs is extremely thin*/,
					Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogCircle: '%s' - Center: (%s) | UpAxis: (%s) | Radius: %f")	, *Command.Text, *Command.Location.ToString(), *Command.Direction.ToString(), Command.Radius);
				}
				break;
			}
			
		case Line:
			{
				if(DrawDebugShapes)
				{
					DrawDebugLine(GetWorld(), Command.Location, Command.End, Command.Color.ToFColor(true),
						false, Command.Lifetime, Command.DepthPriority, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				/**There is no line for the visual logger. Fake it with a really thin capsule*/
				FVisualLogger::CapsuleLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, (Command.End - Command.Location).Size(), Command.Thickness, FQuat::MakeFromRotator(FRotationMatrix::MakeFromZ(Command.End - Command.Location).Rotator()), Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogLine: '%s' - Start: (%s) | End: %s"), *Command.Text, *Command.Location.ToString(), *Command.End.ToString());
				}
				break;
			}
			
		case Box:
			{
				if(DrawDebugShapes)
				{
					DrawDebugBox(GetWorld(), Command.Location, Command.Extent, Command.Color.ToFColor(true),
						false, Command.Lifetime, Command.DepthPriority, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				FBox Box = FBox(Command.Location - Command.Extent, Command.Location + Command.Extent);
				FVisualLogger::BoxLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Box, FMatrix::Identity, 
					Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogBox: '%s' - BoxMin: (%s) | BoxMax: (%s)"), *Command.Text, *Box.Min.ToString(), *Box.Max.ToString());
				}
				break;
			}
			
		case Sphere:
			{
				if(DrawDebugShapes)
				{
					DrawDebugSphere(GetWorld(), Command.Location, Command.Radius, 16, Command.Color.ToFColor(true),
						false, Command.Lifetime, Command.DepthPriority, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				FVisualLogger::SphereLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, Command.Radius, Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogSphere: '%s' - Center: (%s) | Radius: %f"), *Command.Text, *Command.Location.ToString(), Command.Radius);
				}
				break;
			}
			
		case Capsule:
			{
				if(DrawDebugShapes)
				{
					/**We have to offset the center, because Vislog capsule does not use the center. And I think this is far more useful*/
					DrawDebugCapsule(GetWorld(), FVector(Command.Location.X, Command.Location.Y, Command.Location.Z + Command.HalfHeight), Command.HalfHeight, Command.Radius, Command.Rotation, Command.Color.ToFColor(true),
						false, Command.Lifetime, 0, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				FVisualLogger::CapsuleLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, Command.HalfHeight, Command.Radius, Command.Rotation, Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogCapsule: '%s' - Base: (%s) | HalfHeight: %f | Radius: %f | Rotation: (%s)"), *Command.Text, *Command.Location.ToString(), Command.HalfHeight, Command.Radius, *Command.Rotation.ToString());
				}
				break;
			}
			
		case Arrow:
			{
				if(DrawDebugShapes)
				{
					DrawDebugDirectionalArrow(GetWorld(), Command.Location, Command.End, Command.ArrowSize, Command.Color.ToFColor(true), false, Command.Lifetime, Command.DepthPriority, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				FVisualLogger::ArrowLineLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log,
					Command.Location, Command.End, Command.Color.ToFColor(true), Command.ArrowSize, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogText: '%s'"), *Command.Text);
				}
				break;
			}
			
		case Text:
			{
				if(DrawDebugShapes)
				{
					DrawDebugString(GetWorld(), Command.Location, Command.Text, nullptr, Command.Color.ToFColor(true), Command.Lifetime, false, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				/**There is no "Log text" in a way that draws text in the world like we have for the other shapes.
				 * Fake it by making a sphere with 0 radius. This lets us hijack the text system that comes with
				 * other shapes */
				FVisualLogger::SphereLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, 0, Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogText: '%s'"), *Command.Text);
				}
				break;
			}
			
		case Cone:
			{
				if(DrawDebugShapes)
				{
					DrawDebugCone(GetWorld(), Command.Location, Command.Direction, Command.Length, FMath::DegreesToRadians(Command.AngleWidth), FMath::DegreesToRadians(Command.AngleHeight), 
					16, Command.Color.ToFColor(true), false, Command.Lifetime, Command.DepthPriority, Command.Thickness);
				}
				#if ENABLE_VISUAL_LOG
				FVisualLogger::ConeLogf(Command.Owner.Get(), Command.LogCategory, ELogVerbosity::Log
					, Command.Location, Command.Direction, Command.Length, Command.AngleHeight, Command.Color.ToFColor(true), Command.Wireframe, TEXT("%s"), *Command.Text);
				#endif
				if(Command.AddMessageToLog)
				{
					VLOG_BP_LIBRARY_ADD_TO_LOG(Command.LogCategory, TEXT("LogCone: '%s' - Origin: (%s) | Direction: (%s) | Length: %f | Angle: %f"), *Command.Text, *Command.Location.ToString(), *Command.Direction.ToString(), Command.Length, Command.AngleHeight);
				}
				
				break;
			}
		}
		
		Command.Lifetime -= DeltaTime;
		if(Command.Lifetime <= 0.0f)
		{
			ShapesToRemove.Add(CurrentCommand.Key);
		}
	}
	
	for(auto& ShapeToRemove : ShapesToRemove)
	{
		ShapesToDraw.Remove(ShapeToRemove);
	}
}
