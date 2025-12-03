// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Core/FunctionalTestParents/TestAssistantComponent.h"

#include "OmniRuntimeMacros.h"
#include "OmniToolboxVanguard.h"
#include "TraceUtilLibrary.h"
#include "Camera/CameraActor.h"
#include "Core/FunctionalTestParents/VanguardFunctionalTestSubsystem.h"
#include "Engine/DebugCameraController.h"
#include "FunctionLibraries/OmniEditorLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/TraceScreenshot.h"
#include "GameFramework/Pawn.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "VisualLogger/VisualLogger.h"

#if FunctionalTestingEnabled
#include "FunctionalTest.h"
#endif

UTestAssistantComponent::UTestAssistantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTestAssistantComponent::WriteSpecificResult(EVanguardExcelHeader Header, FString ResultText, int32 RowOverride)
{
	if(SpreadsheetObject == nullptr)
	{
		return;
	}
	
	int32 Row = RowOverride < 0 ? RowIndex : RowOverride;
	
	int32 ExpectedResultColumnIndex = SpreadsheetObject->GetColumnIndexByHeader(VanguardSpreadSheet::TestHeaders::ExpectedResult);
	/**Get the value of the "Expected Results" cell within the row we are trying to modify */
	FString LastExpectedResult = SpreadsheetObject->Rows[Row][ExpectedResultColumnIndex].Text;
	
	FString HeaderName;
	switch(Header) {
	case ExpectedResult:
		{
			HeaderName = VanguardSpreadSheet::TestHeaders::ExpectedResult;
			
			/**Check if the current row is populated. Then automatically add
			 * a new row and assign it as the new RowIndex */
			if(LastExpectedResult.IsEmpty() == false)
			{
				StartNewSpreadsheetRow(CurrentTestLog);
				Row = RowOverride < 0 ? RowIndex : RowOverride;
			}
			
			break;
		}
	case ActualResult:
		{
			HeaderName = VanguardSpreadSheet::TestHeaders::ActualResult;
			break;
		}
	}
	
	int32 ColumnIndex = SpreadsheetObject->GetColumnIndexByHeader(HeaderName);
	if(ColumnIndex == INDEX_NONE)
	{
		return;
	}
	
	if(Header == ActualResult)
	{
		/**We've put in an actual result. Paint the cells green if the results match.
		 * If not, paint them red, create an error and try to take a screenshot */
		if(LastExpectedResult != ResultText)
		{
			/**Results don't match, report the issue.*/
			FString ErrorMessage = FString("Result: " + ResultText + " - does not match expected result: " + LastExpectedResult);
			UOmniEditorLibrary::RaiseScriptError(ErrorMessage);
			SpreadsheetObject->ColorCell(Row, ColumnIndex, FLinearColor::Red, false);
			SpreadsheetObject->ColorCell(Row, ExpectedResultColumnIndex, FLinearColor::Red, false);
			ContainsErrorInSpreadsheet = true;
			if(SHOULD_TRACE_SCREENSHOT())
			{
				UTraceUtilLibrary::TraceScreenshot(ErrorMessage, false);
			}
		}
		else
		{
			SpreadsheetObject->ColorCell(Row, ColumnIndex, FLinearColor::Green, false);
			SpreadsheetObject->ColorCell(Row, ExpectedResultColumnIndex, FLinearColor::Green, false);
		}
	}
	

	/**Finally perform the cell edit operation */
	SpreadsheetObject->EditCell(Row, ColumnIndex, ResultText, false);
}

void UTestAssistantComponent::WriteResult(FString ExpectedResult, FString ActualResult, int32 RowOverride)
{
	if(SpreadsheetObject)
	{
		WriteSpecificResult(EVanguardExcelHeader::ExpectedResult, ExpectedResult, RowOverride);
		WriteSpecificResult(EVanguardExcelHeader::ActualResult, ActualResult, RowOverride);
	}
}

void UTestAssistantComponent::StartNewSpreadsheetRow(FString NewSpreadsheetRow)
{
#if FunctionalTestingEnabled
	const FString OldSpreadsheetRow = CurrentTestLog;
	if(SpreadsheetObject)
	{
		const AFunctionalTest* TestActor = Cast<AFunctionalTest>(GetOwner());
		CurrentTestLog = NewSpreadsheetRow;
		/**Add a new row and assign the test label as the tester*/
		RowIndex = SpreadsheetObject->AddRowByName(TestActor->TestLabel, false);
		/**Assign the TestName into the appropriate column*/
		SpreadsheetObject->EditCell(RowIndex, SpreadsheetObject->GetColumnIndexByHeader(VanguardSpreadSheet::TestHeaders::TestName), NewSpreadsheetRow, false);
	}
	if(AutomaticallyStartInsightsTrace)
	{
		#if UE_TRACE_ENABLED
		{
			TRACE_END_REGION(*OldSpreadsheetRow)
			
			TRACE_BEGIN_REGION(*NewSpreadsheetRow);
		}
		#endif
	}
#endif
}


// Called when the game starts
void UTestAssistantComponent::BeginPlay()
{
	Super::BeginPlay();

#if FunctionalTestingEnabled
	
	AFunctionalTest* TestActor = Cast<AFunctionalTest>(GetOwner());
	if(TestActor == nullptr)
	{
		UE_LOG(LogVanguard, Error, TEXT("TestAssistantComponent attached to actor %s that is not a functional test."), *GetOwner()->GetName());
		return;
	}
	
	for (FConstPlayerControllerIterator PCIterator = GetWorld()->GetPlayerControllerIterator(); PCIterator; ++PCIterator)
	{
		APlayerController* PC = PCIterator->Get();

		// Don't use debug camera player controllers.
		// While it's tempting to teleport the camera if the user is debugging something then moving the camera around will them.
		if (PC && !PC->IsA(ADebugCameraController::StaticClass()))
		{
			TargetPC = PC;
			break;
		}
	}

	if (TargetPC.IsValid())
	{
		BeginPlayLocation = GetOwner()->GetActorLocation();
	}
	
	TestActor->OnTestPrepare.AddDynamic(this, &UTestAssistantComponent::OnTestPrepare);
	TestActor->OnTestStart.AddDynamic(this, &UTestAssistantComponent::OnTestStarted);
	TestActor->OnTestFinished.AddDynamic(this, &UTestAssistantComponent::OnTestFinished);
#endif
}


void UTestAssistantComponent::OnTestPrepare()
{
#if FunctionalTestingEnabled
	if(UVanguardFunctionalTestSubsystem* Subsystem = GetWorld()->GetSubsystem<UVanguardFunctionalTestSubsystem>())
	{
		if(AutomaticallyRecordVislog)
		{
			UOmniEditorLibrary::EnableVislogRecordingToFile(true);	
		}
		
		/**Make a new spreadsheet with this class's name.
		 * Even if we don't save it, it will help us catch issues
		 * when we use the @WriteResult function and results did
		 * not match. */
		SpreadsheetObject = NewObject<UOmniSpreadsheetObject>(this);
		SpreadsheetObject->Initialize(TArray<FString>{
			VanguardSpreadSheet::TestHeaders::TesterName,
			VanguardSpreadSheet::TestHeaders::TestName,
			VanguardSpreadSheet::TestHeaders::ExpectedResult,
			VanguardSpreadSheet::TestHeaders::ActualResult
			},
			Subsystem->FilepathToUse,
			GetOwner()->GetClass()->GetAuthoredName());
		
		Subsystem->TestBeginning(this);
	}
	
	bool LocationChanged = false;
	
	AActor* TargetObservationPoint = nullptr;
	
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("DefaultCamera"), OutActors);
	if(OutActors.IsValidIndex(0))
	{
		TargetObservationPoint = OutActors[0];
	}
	
	TArray<AActor*> AttachedActors;
	GetOwner()->GetAttachedActors(AttachedActors);
	if(AttachedActors.IsValidIndex(0))
	{
		for(auto& Actor : AttachedActors)
		{
			if(Actor && Cast<ACameraActor>(Actor))
			{
				TargetObservationPoint = Actor;
				break;
			}
		}
	}
	
	if(UseDefaultCameraIfNoObservationPointIsSet)
	{
		if(TargetPC.IsValid())
		{
			if(BeginPlayLocation.Equals(TargetPC->GetPawn()->GetActorLocation()))
			{
				/**The test is still in the same location as it was on BeginPlay, meaning the observation point
				 * is most likely not set. Find a default camera and move the player controller there. */
				
				
				if(TargetPC.IsValid())
				{
					if (TargetPC->GetPawn())
					{
						TargetPC->GetPawn()->TeleportTo(TargetObservationPoint->GetActorLocation(), TargetObservationPoint->GetActorRotation(), /*bIsATest=*/false, /*bNoCheck=*/true);
						TargetPC->SetControlRotation(TargetObservationPoint->GetActorRotation());
					}
					else
					{
						TargetPC->SetViewTarget(TargetObservationPoint);
					}
				}
			}
		}
	}
	
	if(AutomaticallyStartInsightsTrace)
	{
#if UE_TRACE_ENABLED
		FTraceAuxiliary::FOptions TracingOptions;
		TracingOptions.bExcludeTail = true;  // optional: exclude pre-trace buffer history

		FString TraceFileName = FString::Printf(TEXT("%s"), *GetOwner()->GetClass()->GetAuthoredName());
		bool IsTracing = FTraceAuxiliary::Start(
			FTraceAuxiliary::EConnectionType::File,
			nullptr,
			nullptr,
			&TracingOptions
		);
		
		UE_LOG(LogVanguard, Log, TEXT("Is Tracing set to: %s"), IsTracing ? *FString("True") : *FString("False"));
#endif
	}
	
	if(TargetPC.IsValid())
	{
		for(auto& Command : ConsoleCommandsToExecuteOnPrepareTest)
		{
			TargetPC->ConsoleCommand(Command);
		}
	}

#endif
}

void UTestAssistantComponent::OnTestStarted()
{
#if FunctionalTestingEnabled
#endif
}

void UTestAssistantComponent::OnTestFinished()
{
#if FunctionalTestingEnabled
	if(UVanguardFunctionalTestSubsystem* Subsystem = GetWorld()->GetSubsystem<UVanguardFunctionalTestSubsystem>())
	{
		AFunctionalTest* TestActor = Cast<AFunctionalTest>(GetOwner());
		if(FVisualLogger::Get().IsRecordingToFile())
		{
			Omni_InsightsTrace_Append("EndVislog")
			UOmniEditorLibrary::EnableVislogRecordingToFile(false); 
			
			/**There's no real clean way of accessing the file that Vislog just wrote to.
			 * Find the latest log that has been generated, then if it's found,
			 * move it and rename it on the way. */
			const FString VisLogDir = FPaths::ProjectSavedDir() / TEXT("Logs");
			TArray<FString> FoundFiles;
			IFileManager::Get().FindFiles(FoundFiles, *(VisLogDir / TEXT("*.bvlog")), true, false);
		
			FString LatestFile;
			FDateTime LatestTime;
		
			for (const FString& File : FoundFiles)
			{
				const FString FullPath = VisLogDir / File;
				const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*FullPath);
		
				if (Timestamp > LatestTime)
				{
					LatestTime  = Timestamp;
					LatestFile = FullPath;
				}
			}
		
			if(LatestFile.IsEmpty() == false)
			{
				IFileManager::Get().Move(*(FPaths::ProjectSavedDir() / Subsystem->FilepathToUse / (FString::Printf(TEXT("%s_Vislog.bvlog"), *GetOwner()->GetClass()->GetAuthoredName()))), *LatestFile);
			}
		}
		
		if(SpreadsheetObject)
		{
			Omni_InsightsTrace_Append("ValidateSpreadsheet")
			/**Check for any empty ExpectedResult rows*/
			int32 ExpectedResultColumn = SpreadsheetObject->GetColumnIndexByHeader(VanguardSpreadSheet::TestHeaders::ExpectedResult);
			for(int32 i = 1; i < SpreadsheetObject->GetNumberOfRows(); i++)
			{
				FOmniSpreadsheetCell Cell = SpreadsheetObject->GetCellData(i, ExpectedResultColumn);
				if(Cell.Text.IsEmpty())
				{
					UOmniEditorLibrary::RaiseScriptError(FString("Expected Result cell is empty in row " + FString::FromInt(i)));
					TestActor->AddError(FString("Expected Result cell is empty in row " + FString::FromInt(i)));
					TestActor->Result = EFunctionalTestResult::Failed;
				}
			}
		
			if(ContainsErrorInSpreadsheet)
			{
				TestActor->AddError(FString("Spreadsheet contains errors. See log for details."));
				TestActor->Result = EFunctionalTestResult::Failed;
			}
			
			if(GenerateSpreadsheets)
			{
				SpreadsheetObject->FilePath.RemoveFromEnd(".html");
				SpreadsheetObject->FilePath += FString("_") + LexToString(TestActor->Result) + ".html";
				SpreadsheetObject->Save();
			}
		}
		
		if(TargetPC.IsValid())
		{
			for(auto& Command : ConsoleCommandsToExecuteOnPrepareTest)
			{
				TargetPC->ConsoleCommand(Command);
			}
		}
		
		if(AutomaticallyStartInsightsTrace)
		{
			#if UE_TRACE_ENABLED
			if(FTraceAuxiliary::Stop())
			{
				Omni_InsightsTrace_Append("EndInsightsTrace")
				/**There's no real clean way of accessing the file that Insights just wrote to.
				 * Find the latest log that has been generated, then if it's found,
				 * move it and rename it on the way. */
				const FString ProfilingDirectory = FPaths::ProfilingDir();
				TArray<FString> FoundFiles;
				IFileManager::Get().FindFiles(FoundFiles, *(ProfilingDirectory / TEXT("*.utrace")), true, false);
				
				FString LatestFile;
				FDateTime LatestTime;
		
				for (const FString& File : FoundFiles)
				{
					const FString FullPath = ProfilingDirectory / File;
					const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*FullPath);
		
					if (Timestamp > LatestTime)
					{
						LatestTime  = Timestamp;
						LatestFile = FullPath;
					}
				}
		
				if(LatestFile.IsEmpty() == false)
				{
					Omni_InsightsTrace_Append("EndInsightsTrace::MoveFile")
					IFileManager::Get().Move(*(FPaths::ProjectSavedDir() / Subsystem->FilepathToUse / (FString::Printf(TEXT("%s_Trace.utrace"), *GetOwner()->GetClass()->GetAuthoredName()))), *LatestFile);
				}
		
				// for (const FString& File : FoundFiles)
				// {
				// 	if(File.Contains(*GetOwner()->GetClass()->GetAuthoredName()))
				// 	{
				// 		IFileManager::Get().Move(*(FPaths::ProjectSavedDir() / Subsystem->FilepathToUse / File), *File);
				// 		break;
				// 	}
				// }
			}
			#endif
		}
			
		Subsystem->TestEnding(this, TestActor->Result);
	}

#endif
}


