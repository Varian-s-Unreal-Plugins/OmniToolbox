// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Core/FunctionalTestParents/VanguardFunctionalTestSubsystem.h"

#include "FunctionalTestingManager.h"
#include "FunctionalTestingModule.h"
#include "OmniRuntimeMacros.h"
#include "OmniToolboxVanguard.h"
#include "VanguardFunctionalTest.h"
#include "VanguardTestingSettings.h"
#include "HAL/PlatformFramePacer.h"
#include "Settings/EditorExperimentalSettings.h"

Omni_ConsoleVariable(OMNITOOLBOXVANGUARD_API, bool, GenerateSpreadsheets, true,
                     "OmniToolbox.Vanguard.GenerateSpreadsheets", "Allows testers to generate a spreadsheet")

void UVanguardFunctionalTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	
	if(GIsAutomationTesting == false)
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
	
	//Timestamped filename
	const FDateTime Now = FDateTime::Now();
	const FString Timestamp = FString::Printf(TEXT("%02d-%02d-%04d_%02d-%02d-%02d"),
		Now.GetDay(), Now.GetMonth(), Now.GetYear(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());
		
	FilepathToUse = VanguardSpreadSheet::Directory / Timestamp;
	
#endif
	
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

void UVanguardFunctionalTestSubsystem::TestBeginning(AVanguardFunctionalTest* Test)
{
	CurrentTest = Test;
	
	if(PerformanceSpreadsheet)
	{
		CurrentTestRow = PerformanceSpreadsheet->AddRowByName(Test->TestLabel, false);
	}
}

void UVanguardFunctionalTestSubsystem::TestEnding(AVanguardFunctionalTest* Test, EFunctionalTestResult FinishState)
{
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
}

void UVanguardFunctionalTestSubsystem::Tick(float DeltaTime)
{
	if(PerformanceSpreadsheet == nullptr)
	{
		Super::Tick(DeltaTime);
		return;
	}
	
	const UVanguardTestingSettings* Settings = GetDefault<UVanguardTestingSettings>();
	
	if(DeltaTime > (1 / Settings->HitchDeltaThreshold) && PerformanceSpreadsheet && CurrentTest.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hitch detected in the test. DeltaTime: %f"), DeltaTime);
		CurrentTest.Get()->HitchesDetected++;
		int32 HitchesColumn = PerformanceSpreadsheet->GetColumnIndexByHeader("Hitches");
		PerformanceSpreadsheet->EditCell(GetRow(), HitchesColumn, FString::FromInt(CurrentTest.Get()->HitchesDetected));
	}
	
	Frames.Add(DeltaTime);
	
	Super::Tick(DeltaTime);
}
