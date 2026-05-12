// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Developer/SpreadsheetHelpers/OmniSpreadsheetObject.h"
#include "TestAssistantComponent.generated.h"

UENUM(BlueprintType)
enum EVanguardExcelHeader : uint8
{
	ExpectedResult,
	ActualResult
};

enum EVanguardExcelHeader : uint8;
/**This component is designed to go on functional test actors
 * and provide them with utilities to improve automation testing. */
UCLASS(ClassGroup=(Vanguard), meta=(BlueprintSpawnableComponent))
class OMNITOOLBOXVANGUARD_API UTestAssistantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTestAssistantComponent();
	
	/**Because ObservationPoint is inaccessible (Epic sure love their protected/private...
	 * We can only find out if the test does NOT have an observation point by checking
	 * if it has moved from its original location. */
	FVector BeginPlayLocation;
	
	/**If there's no observation point assigned, we will look for
	 * any actor with the tag "DefaultCamera" and assign that
	 * as our observation point */
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	bool UseDefaultCameraIfNoObservationPointIsSet = true;
	
	UPROPERTY(Category = "Functional Testing", BlueprintReadWrite)
	TObjectPtr<UOmniSpreadsheetObject> SpreadsheetObject = nullptr;
	
	/**When the test is started, should we record a Vislog session?*/
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	bool AutomaticallyRecordVislog = false;
	
	/**When the test is preparing to start, should we start an Unreal Insights trace?*/
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	bool AutomaticallyStartInsightsTrace = false;
	
	/**When we are preparing the test (before @StartTest is called)
	 * we will execute these console commands */
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	TArray<FString> ConsoleCommandsToExecuteOnPrepareTest;
	
	/**When we are ending the test, we will execute these console commands */
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	TArray<FString> ConsoleCommandsToExecuteOnEndTest;
	
	FString CurrentTestLog;
	
	/**Write a result into the spreadsheet (if enabled) and put
	 * the @Result into the @Header column.
	 * This is designed to be used in scenarios where you have an event
	 * that writes down the actual result.
	 * For example, adding a tag to an entity, you would write down the
	 * expected result. Then, on some event, you would write the actual
	 * result. This will then help you catch scenarios where the event
	 * you were expecting to trigger might not trigger.
	 * 
	 * @RowOverride Write to a specific row. When doing tests with
	 * multiple latent results, you can use this to offset from the
	 * current row the spreadsheet is editing. */
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable)
	void WriteSpecificResult(EVanguardExcelHeader Header, FString ResultText, int32 RowOverride = -1);
	
	/**Write a result into the spreadsheet (if enabled) and put
	 * the @Result into the @Header column.
	 * This is designed to be used in scenarios where you have an event
	 * that writes down the actual result.
	 * For example, adding a tag to an entity, you would write down the
	 * expected result. Then, on some event, you would write the actual
	 * result. This will then help you catch scenarios where the event
	 * you were expecting to trigger might not trigger.
	 * 
	 * @RowOverride Write to a specific row. When doing tests with
	 * multiple latent results, you can use this to offset from the
	 * current row the spreadsheet is editing. */
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable)
	void WriteResult(FString ExpectedResult, FString ActualResult, int32 RowOverride = -1);
	
	/**Only used for the spreadsheet system.
	 * Many tests have a label, but then they perform multiple
	 * sub-tests inside of them. This helps distinguish between them
	 * and gives each sub-test its own row inside the spreadsheet.
	 * For example; applying a gameplay effect on a target might
	 * be the "Apply GE" test and you'd call this and label it as
	 * "Apply GE to target". Then, we want to monitor changes
	 * made by that effect. So you'd call this function again
	 * and label it as "Target Health changed" */
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable)
	void StartNewSpreadsheetRow(FString NewSpreadsheetRow);
	
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable, BlueprintPure)
	int32 GetRowIndex(bool IncludeOffset) const
	{
		if(IncludeOffset && IsValid(SpreadsheetObject))
		{
			return RowIndex * SpreadsheetObject->GetNumberOfRows();
		}
		return RowIndex;
	}
	
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable, BlueprintPure)
	int32 GetNumberOfActualResults();
	
	/**This is stored in here for simplicity, but it's NOT modified
	 * by this actor. It's modified by the subsystem.
	 * It's done like this because we might not want these tests to
	 * tick, and some tests might override the tick and accidentally
	 * forget to include the parent function. */
	int32 HitchesDetected = 0;
	
	bool ContainsErrorInSpreadsheet = false;
	
	TWeakObjectPtr<APlayerController> TargetPC = nullptr;

private:
	int32 RowIndex = 0;
	int32 RowOffset = 0;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnTestPrepare();
	
	UFUNCTION()
	virtual void OnTestStarted();
	
	UFUNCTION()
	virtual void OnTestFinished();
};
