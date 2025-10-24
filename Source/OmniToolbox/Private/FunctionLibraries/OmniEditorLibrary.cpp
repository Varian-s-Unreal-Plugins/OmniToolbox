// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniEditorLibrary.h"

#include "Blueprint/BlueprintExceptionInfo.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#if WITH_EDITOR
#include "Engine/Console.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/BlueprintEditorUtils.h"
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
		const FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::UserRaisedError, FText::FromString(ErrorMessage));
		FBlueprintCoreDelegates::ThrowScriptException(TopFrame->Object, *TopFrame, ExceptionInfo);
#else
		UE_LOG(LogBlueprintUserMessages, Error, TEXT("%s:\n%s"), *ErrorMessage, *TopFrame->GetStackTrace());
#endif	// WITH_EDITOR
	}
#endif	// !(UE_BUILD_TEST || UE_BUILD_SHIPPING)
}
