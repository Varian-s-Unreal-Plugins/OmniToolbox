// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Core/FunctionalTestParents/VanguardFunctionalTestSubsystem.h"

#include "OmniRuntimeMacros.h"
#include "OmniToolboxVanguard.h"
#include "TestAssistantComponent.h"
#if FunctionalTestingEnabled
#include "FunctionalTest.h"
#endif
#include "VanguardTestingSettings.h"
#if WITH_EDITOR
#include "Settings/EditorExperimentalSettings.h"
#endif


Omni_ConsoleVariable(OMNITOOLBOXVANGUARD_API, bool, GenerateSpreadsheets, false,
                     "OmniToolbox.Vanguard.GenerateSpreadsheets", "Allows testers to generate a spreadsheet")

Omni_ConsoleVariable(OMNITOOLBOXVANGUARD_API, bool, OverrideIsAutomationTesting, false,
					 "OmniToolbox.Vanguard.OverrideIsAutomationTesting", "Override the GIsAutomationTesting checks to allow Vanguard to function without")

void UVanguardFunctionalTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	/**V: Note: If we change @GIsAutomationTesting, the screen just goes black and it
	 * seems that the application crashes. Hence why we aren't just simply changing
	 * the value of it. */
	if(GIsAutomationTesting == false && OverrideIsAutomationTesting == false)
	{
		Super::Initialize(Collection);
		return;
	}
	
	IsRunning = true;
	
#if WITH_EDITOR
	/**We must make sure that bBreakOnExceptions is false while running tests,
	 * because this is supposed to be automated and ideally, ran on some other
	 * machine and you do not have to babysit it.
	 * If this setting is enabled, you must manually press resume.
	 * This can mean that if you're running a lot of tests overnight,
	 * you might not wake up to all the tests having completed because
	 * one test had an error. */
	UEditorExperimentalSettings* ExperimentalSettings = GetMutableDefault<UEditorExperimentalSettings>();
	OriginalBreakOnBlueprintExceptionSetting = ExperimentalSettings->bBreakOnExceptions;
	ExperimentalSettings->bBreakOnExceptions = false;
#endif
	
	//Timestamped filename
	const FDateTime Now = FDateTime::Now();
	const FString Timestamp = FString::Printf(TEXT("%02d-%02d-%04d_%02d-%02d-%02d"),
		Now.GetDay(), Now.GetMonth(), Now.GetYear(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());
		
	FilepathToUse = VanguardSpreadSheet::Directory / Timestamp;
	
	if(GenerateSpreadsheets)
	{
		UE_LOG(LogVanguard, Warning, TEXT("Generating spreadsheets"));		
		
		PerformanceSpreadsheet = NewObject<UOmniSpreadsheetObject>(this);
		
		/**TODO: Add these into its own table above the one we're about to generate */
		// "Hardware", 
		// "Operating System",
		// "Scalability"
		PerformanceSpreadsheet->Initialize(TArray<FString>
		{
			VanguardSpreadSheet::PerformanceHeaders::TestName, 
			VanguardSpreadSheet::PerformanceHeaders::FPS, 
			VanguardSpreadSheet::PerformanceHeaders::Hitches, 
			VanguardSpreadSheet::PerformanceHeaders::AvgGameThread,
			VanguardSpreadSheet::PerformanceHeaders::TestTime,
			VanguardSpreadSheet::PerformanceHeaders::Status
		},
			GenerateSpreadsheets == false,
			FilepathToUse);
	}
	
	Super::Initialize(Collection);
}

void UVanguardFunctionalTestSubsystem::Deinitialize()
{
	if(IsRunning == false)
	{
		Super::Deinitialize();
		return;
	}
	
	if(GlobalTestSpreadsheet)
	{
		GlobalTestSpreadsheet->Save();
	}
	
	if(PerformanceSpreadsheet)
	{
		PerformanceSpreadsheet->Save();
	}
	
#if WITH_EDITOR
	/**Restore the setting that was being used*/
	UEditorExperimentalSettings* ExperimentalSettings = GetMutableDefault<UEditorExperimentalSettings>();
	ExperimentalSettings->bBreakOnExceptions = OriginalBreakOnBlueprintExceptionSetting;
#endif
	
	Super::Deinitialize();
}

void UVanguardFunctionalTestSubsystem::TestBeginning(UTestAssistantComponent* Test)
{
#if FunctionalTestingEnabled
	CurrentTest = Test;
	
	if(PerformanceSpreadsheet)
	{
		AFunctionalTest* FunctionalTest = Cast<AFunctionalTest>(Test->GetOwner());
		CurrentTestRow = PerformanceSpreadsheet->AddRowByName(FunctionalTest->TestLabel, false);
	}
#endif
}

void UVanguardFunctionalTestSubsystem::TestEnding(UTestAssistantComponent* Test, EFunctionalTestResult FinishState)
{
#if FunctionalTestingEnabled
	if(PerformanceSpreadsheet == nullptr)
	{
		CurrentTest = nullptr;
		return;
	}
	
	float TotalTime = 0;
	for(auto& CurrentFrame : Frames)
	{
		TotalTime += CurrentFrame;
	}
	float AverageGameThread = TotalTime / Frames.Num();
	float AverageFPS = 1 / AverageGameThread;
	
	PerformanceSpreadsheet->EditCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::FPS, FString::SanitizeFloat(AverageFPS));
	PerformanceSpreadsheet->EditCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::AvgGameThread, FString::SanitizeFloat(AverageGameThread));
	PerformanceSpreadsheet->EditCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::Hitches, FString::FromInt(Test->HitchesDetected));
	PerformanceSpreadsheet->EditCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::TestTime, FString::SanitizeFloat(TotalTime));
	
	/**Final status column*/
	PerformanceSpreadsheet->EditCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::Status, LexToString(FinishState));
	if(FinishState == EFunctionalTestResult::Succeeded || FinishState == EFunctionalTestResult::Default)
	{
		PerformanceSpreadsheet->ColorCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::Status, FLinearColor::Green);
	}
	else
	{
		PerformanceSpreadsheet->ColorCellByColumnName(CurrentTestRow, VanguardSpreadSheet::PerformanceHeaders::Status, FLinearColor::Red);
	}
	
	CurrentTest = nullptr;
	Frames.Empty();
#endif
}

void UVanguardFunctionalTestSubsystem::Tick(float DeltaTime)
{
#if FunctionalTestingEnabled
	if(PerformanceSpreadsheet == nullptr)
	{
		Super::Tick(DeltaTime);
		return;
	}
	
	const UVanguardTestingSettings* Settings = GetDefault<UVanguardTestingSettings>();
	
	if(CurrentTest.IsValid())
	{
		float FrameDifference = DeltaTime - LastDeltaTime;
		if(FrameDifference > Settings->HitchDeltaThreshold && PerformanceSpreadsheet && CurrentTest.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hitch detected in the test. DeltaTime: %f"), DeltaTime);
			CurrentTest.Get()->HitchesDetected++;
			int32 HitchesColumn = PerformanceSpreadsheet->GetColumnIndexByHeader("Hitches");
			PerformanceSpreadsheet->EditCell(GetRow(), HitchesColumn, FString::FromInt(CurrentTest.Get()->HitchesDetected));
		}
	
		LastDeltaTime = DeltaTime;
	
		Frames.Add(DeltaTime);
	}
#endif
	Super::Tick(DeltaTime);
}
