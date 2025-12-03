// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VanguardTestingSettings.h"
#include "Developer/SpreadsheetHelpers/OmniSpreadsheetObject.h"
#include "Subsystems/WorldSubsystem.h"
#include "VanguardFunctionalTestSubsystem.generated.h"

class UTestAssistantComponent;
enum class EFunctionalTestResult : uint8;
class AVanguardFunctionalTest;

extern OMNITOOLBOXVANGUARD_API bool GenerateSpreadsheets;

namespace VanguardSpreadSheet
{
	inline FString Directory = "Automation/Vanguard";
	
	namespace TestHeaders
	{
		inline FString TesterName = "Tester";
		inline FString TestName = "Test";
		inline FString ActualResult = "Actual Result";
		inline FString ExpectedResult = "Expected Result";
	}
	
	namespace PerformanceHeaders
	{
		inline FString Hardware = "Hardware";
		inline FString OperatingSystem = "Operating System";
		inline FString TestName = "Test name";
		inline FString FPS = "FPS";
		inline FString Hitches = "Hitches";
		inline FString AvgGameThread = "Avg Game Thread";
		inline FString TestTime = "Test Time";
		inline FString Status = "Status";
	}
}

/**
 * 
 */
UCLASS()
class OMNITOOLBOXVANGUARD_API UVanguardFunctionalTestSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TObjectPtr<UOmniSpreadsheetObject> PerformanceSpreadsheet = nullptr;
	
	UPROPERTY()
	TObjectPtr<UOmniSpreadsheetObject> GlobalTestSpreadsheet = nullptr;
	
	FVanguardScalabilityPerfTestSettings PerformanceSettings;
	
	/**Row 1 = Hardware/OS/Scalability
	 * Row 2 = Results of row 2
	 * Row 3 = Spacing
	 * Row 4 = Spacing
	 * Row 5 = The headers of the performance sheet */
	int32 RowOffset = 5;
	
	int32 GetRow() const { return RowOffset + RowOffset; }
	int32 CurrentTestRow = 0;
	
	/**By default, just put it into the spreadsheets folder.
	 * This is modified inside the @Initialize function */
	FString FilepathToUse = VanguardSpreadSheet::Directory;
	
	TWeakObjectPtr<UTestAssistantComponent> CurrentTest = nullptr;
	
	bool OriginalBreakOnBlueprintExceptionSetting = false;
	bool IsRunning = false;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void TestBeginning(UTestAssistantComponent* Test);
	void TestEnding(UTestAssistantComponent* Test, EFunctionalTestResult FinishState);

	virtual TStatId GetStatId() const override { return TStatId(); }
	
	TArray<float> Frames;
	
	float LastDeltaTime = 0;
	virtual void Tick(float DeltaTime) override;
};
