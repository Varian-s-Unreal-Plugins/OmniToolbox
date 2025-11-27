// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniEditorLibrary.h"

#include "Blueprint/BlueprintExceptionInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Engine/Console.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/OmniDebugDrawSubsystem.h"
#include "UObject/Stack.h"
#include "VisualLogger/VisualLogger.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "Kismet2/BlueprintEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#endif

bool UOmniEditorLibrary::ImplementInterface(UObject* InstigatingObject, UObject* TargetObject, UClass* Interface, TArray<FString> CustomMessages)
{
	#if WITH_EDITOR

	if(!TargetObject || !Interface)
	{ return false; }

	if(UGameplayStatics::GetGameInstance(TargetObject))
	{
		//Function is not allowed to run during game session
		return false;
	}
	
	if(InstigatingObject->IsTemplate())
	{ return false; }

	if(!TargetObject->GetClass())
	{ return false; }

	TObjectPtr<UObject> BPGC = Cast<UBlueprintGeneratedClass>(TargetObject->GetClass())->ClassGeneratedBy;
	if(!BPGC)
	{ return false; }
	
	UBlueprint* OuterBlueprint = Cast<UBlueprint>(BPGC);
	if(!OuterBlueprint)
	{ return false; }

	if(FBlueprintEditorUtils::ImplementsInterface(OuterBlueprint, true, Interface))
	{
		//Already implements interface
		return false;
	}
	
	FString DialogText = FString::Printf(TEXT("%s requires the implementation of the %s interface to this class.\nThis interface will now automatically be added."),
		*InstigatingObject->GetClass()->GetAuthoredName(), *Interface->GetAuthoredName());

	if(CustomMessages.IsValidIndex(0))
	{
		DialogText += "\n\n";
	}
	for(int32 CurrentIndex = 0; CurrentIndex < CustomMessages.Num(); CurrentIndex++)
	{
		DialogText += "	- " + CustomMessages[CurrentIndex];
		if(CurrentIndex != CustomMessages.Num() - 1)
		{
			//If this isn't the last message, add a new line
			DialogText += "\n\n";
		}
	}
	
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DialogText));
	FBlueprintEditorUtils::ImplementNewInterface(OuterBlueprint, Interface->GetClassPathName());
	return true;

	#endif

	return false;
}

void UOmniEditorLibrary::SetObjectDisplayName(UObject* Object, const FString& NewName)
{
	#if WITH_EDITOR
	if(Object && !Object->IsAsset())
	{
		Object->Rename(*NewName, Object->GetOuter(), REN_DontCreateRedirectors | REN_DoNotDirty);
	}
	#endif
}

void UOmniEditorLibrary::SendNotificationWithState(FString Message, bool Success)
{
#if WITH_EDITOR
	FNotificationInfo Info(FText::FromString(Message));
	Info.ExpireDuration = 8.0f;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(Success ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
#endif
}

void UOmniEditorLibrary::SendNotification(FString Message)
{
#if WITH_EDITOR
	FNotificationInfo Info(FText::FromString(Message));
	Info.ExpireDuration = 8.0f;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	FSlateNotificationManager::Get().AddNotification(Info);
#endif



#if WITH_EDITOR
	// FNotificationInfo Info(FText::FromString(Message));
	// Info.bUseLargeFont = false;
	// Info.bFireAndForget = false;
	// Info.bUseSuccessFailIcons = false;
	// Info.ButtonDetails.Add(FNotificationButtonInfo(
	// 	INVTEXT("ok"),
	// 	INVTEXT("tooltip"),
	// 	FSimpleDelegate::CreateLambda([](const FText& Text)
	// 	{
	// 		
	// 	})));
	// FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_None);
#endif
}

void UOmniEditorLibrary::RaiseScriptError(const FString& Message)
{
#if !(UE_BUILD_TEST || UE_BUILD_SHIPPING)
	FFrame* TopFrame = FFrame::GetThreadLocalTopStackFrame();
	if (TopFrame)
	{
#if WITH_EDITOR
		const FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::UserRaisedError, FText::FromString(Message));
		FBlueprintCoreDelegates::ThrowScriptException(TopFrame->Object, *TopFrame, ExceptionInfo);
#else
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("%s:\n%s"), *Message, *TopFrame->GetStackTrace());
#endif	// WITH_EDITOR
	}
#endif	// !(UE_BUILD_TEST || UE_BUILD_SHIPPING)
}

void UOmniEditorLibrary::AddInstanceComponent(AActor* Actor, UActorComponent* Component)
{
#if WITH_EDITOR
	if(Actor && Component)
	{
		Actor->PreEditChange(nullptr);
		Actor->AddInstanceComponent(Component);
		Actor->PostEditChange();
	}
#endif
}

void UOmniEditorLibrary::SetComponentEditorOnly(UActorComponent* Component, bool NewEditorOnly)
{
#if WITH_EDITOR
	if(Component)
	{
		Component->bIsEditorOnly = NewEditorOnly;
	}
#endif
}

void UOmniEditorLibrary::MakeComponentInvisibleInActorComponents(UActorComponent* Component)
{
#if WITH_EDITOR
	if(Component)
	{
		Component->SetIsVisualizationComponent(true); 
	}
#endif
}

FString UOmniEditorLibrary::GetPluginNameForClass(UClass* Class)
{
#if WITH_EDITOR
	
	if(Class)
	{
		const FString& ModuleDependency = FPackageName::GetShortName(Class->GetPackage()->GetName());
		
		TArray<FModuleStatus> ModuleStatuses;
		const FModuleManager& ModuleManager = FModuleManager::Get();
		ModuleManager.QueryModules(ModuleStatuses);
		for (FModuleStatus& ModuleStatus : ModuleStatuses)
		{
			if (ModuleStatus.bIsLoaded && ModuleStatus.Name == ModuleDependency)
			{
				// this is the module's plugin
				const TSharedPtr<IPlugin>& OwnerPlugin = IPluginManager::Get().GetModuleOwnerPlugin(*ModuleDependency);
				if(OwnerPlugin.IsValid())
				{
					return OwnerPlugin->GetFriendlyName();
				}
			}
		}
	}
	
	return "";
	
#else
	
	return "";
	
#endif
}

void UOmniEditorLibrary::EnableVislogRecordingToFile(bool bEnabled)
{
#if ENABLE_VISUAL_LOG
	FVisualLogger::Get().SetIsRecordingToFile(bEnabled);
	/**Oddly enough, setting SetIsRecordingToFile does NOT
	 * stop the actual recording. Maybe a bug? */
	if(bEnabled == false)
	{
		FVisualLogger::Get().SetIsRecording(false);
	}
#endif // ENABLE_VISUAL_LOG
}

void UOmniEditorLibrary::DrawAndLogCapsule(UObject* WorldContextObject, FVector Center, float HalfHeight, float Radius,
                                           FQuat Rotation, FString Key, FString Text, FLinearColor Color, FName LogCategory, float Lifetime,
                                           bool bAddToMessageLog, bool bWireframe, EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Key, Thickness, Lifetime, Center, Rotation, HalfHeight, Radius]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Capsule;
			DrawCommand.Location = Center;
			DrawCommand.Rotation = Rotation;
			DrawCommand.HalfHeight = HalfHeight;
			DrawCommand.Radius = Radius;
			DrawCommand.Thickness = Thickness;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
}

void UOmniEditorLibrary::DrawAndLogLine(UObject* WorldContextObject, FVector Start, FVector End, FString Key, FString Text,
	FLinearColor Color, FName LogCategory, float Lifetime, bool bAddToMessageLog, bool bWireframe,
	EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Key, Thickness, Lifetime, Start, End]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Line;
			DrawCommand.Location = Start;
			DrawCommand.End = End;
			DrawCommand.Thickness = Thickness;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
}

void UOmniEditorLibrary::DrawAndLogCone(UObject* WorldContextObject, FVector Start, FVector Direction, float Length, float Angle, FString Key,
                                        FString Text, FLinearColor Color, FName LogCategory, float Lifetime, bool bAddToMessageLog,
                                        bool bWireframe, EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Key, Thickness, Lifetime, Start, Direction, Length, Angle]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Cone;
			DrawCommand.Location = Start;
			DrawCommand.Direction = Direction;
			DrawCommand.Length = Length;
			DrawCommand.AngleHeight = Angle;
			DrawCommand.AngleWidth = Angle;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.Thickness = Thickness;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}	
	});
}

void UOmniEditorLibrary::DrawAndLogCircle(UObject* WorldContextObject, FVector Center, FVector UpAxis, float Radius, FString Key,
	FString Text, FLinearColor Color, FName LogCategory, float Lifetime, bool bAddToMessageLog,
	bool bWireframe, EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Key, Thickness, Center, UpAxis, Radius, Lifetime]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Circle;
			DrawCommand.Location = Center;
			DrawCommand.Rotation = FQuat(UpAxis.Rotation());
			DrawCommand.Radius = Radius;
			DrawCommand.Color = Color;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.Text = Text;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawCommand.Thickness = Thickness;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
}

void UOmniEditorLibrary::DrawAndLogBox(UObject* WorldContextObject, FVector Center, FVector Extent, FString Text,
	FLinearColor Color, FName LogCategory, FName Key, float Lifetime, bool bAddToMessageLog, bool bWireframe,
	EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Lifetime, Key, Thickness, Center, Extent]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Box;
			DrawCommand.Location = Center;
			DrawCommand.Extent = Extent;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawCommand.Thickness = Thickness;
			DrawSubsystem->AddShape(DrawCommand, Key);
		}
	});
}

void UOmniEditorLibrary::DrawAndLogSphere(UObject* WorldContextObject, FVector Center, float Radius, FString Key, FString Text,
	FLinearColor Color, FName LogCategory, float Lifetime, bool bAddToMessageLog, bool bWireframe,
	EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Center, Radius, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Thickness, Key, Lifetime]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Sphere;
			DrawCommand.Location = Center;
			DrawCommand.Radius = Radius;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawCommand.Thickness = Thickness;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
}

void UOmniEditorLibrary::DrawAndLogArrow(UObject* WorldContextObject, FVector Start, FVector End, float ArrowSize, FString Key,
	FString Text, FLinearColor Color, FName LogCategory, float Lifetime, bool bAddToMessageLog,
	bool bWireframe, EDrawDebugSceneDepthPriorityGroup DepthPriority, float Thickness)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Thickness, Key, Start, End, ArrowSize, Lifetime]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Arrow;
			DrawCommand.Location = Start;
			DrawCommand.End = End;
			DrawCommand.ArrowSize = ArrowSize;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawCommand.Thickness = Thickness;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
	
}

void UOmniEditorLibrary::DrawAndLogText(UObject* WorldContextObject, FVector Location, FString Text, FString Key, FLinearColor Color,
	FName LogCategory, float Lifetime, bool bAddToMessageLog, bool bWireframe,
	EDrawDebugSceneDepthPriorityGroup DepthPriority, float FontSize)
{
	AsyncTask(ENamedThreads::GameThread, [WorldContextObject, Color, Text, bAddToMessageLog, bWireframe, LogCategory, DepthPriority, Lifetime, FontSize, Key, Location]()
	{
		if(UOmniDebugDrawSubsystem* DrawSubsystem = WorldContextObject->GetWorld()->GetSubsystem<UOmniDebugDrawSubsystem>())
		{
			FOmniDebugDrawCommand DrawCommand;
			DrawCommand.Owner = WorldContextObject;
			DrawCommand.Type = EOmniDebugDrawType::Text;
			DrawCommand.Location = Location;
			DrawCommand.Color = Color;
			DrawCommand.Text = Text;
			DrawCommand.AddMessageToLog = bAddToMessageLog;
			DrawCommand.Lifetime = Lifetime;
			DrawCommand.Wireframe = bWireframe;
			DrawCommand.LogCategory = LogCategory;
			DrawCommand.DepthPriority =  DepthPriority == EDrawDebugSceneDepthPriorityGroup::World ? ESceneDepthPriorityGroup::SDPG_World : ESceneDepthPriorityGroup::SDPG_Foreground;
			DrawCommand.Thickness = FontSize;
			DrawSubsystem->AddShape(DrawCommand, FName(Key));
		}
	});
}
