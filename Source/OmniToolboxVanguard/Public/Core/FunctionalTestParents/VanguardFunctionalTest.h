// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "OmniRuntimeMacros.h"
#include "Developer/SpreadsheetHelpers/OmniSpreadsheetObject.h"
#include "VanguardFunctionalTest.generated.h"

UENUM(BlueprintType)
enum EVanguardExcelHeader : uint8
{
	ExpectedResult,
	ActualResult
};

/**OmniToolbox's parent for all functional tests. */
UCLASS(Abstract)
class OMNITOOLBOXVANGUARD_API AVanguardFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	
	UPROPERTY(Category = "Functional Testing", BlueprintReadWrite)
	TObjectPtr<UOmniSpreadsheetObject> SpreadsheetObject = nullptr;
	
	/**When the test is started, should we record a Vislog session?*/
	UPROPERTY(Category = "Functional Testing", BlueprintReadOnly, EditAnywhere)
	bool AutomaticallyRecordVislog = false;
	
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
	void StartNewTest(FString InTestName);
	
	UFUNCTION(Category = "Vanguard Functional Test", BlueprintCallable, BlueprintPure)
	int32 GetRowIndex(bool IncludeOffset) const
	{
		if(IncludeOffset && IsValid(SpreadsheetObject))
		{
			return RowIndex * SpreadsheetObject->GetNumberOfRows();
		}
		return RowIndex;
	}
	
	/**This is stored in here for simplicity, but it's NOT modified
	 * by this actor. It's modified by the subsystem.
	 * It's done like this because we might not want these tests to
	 * tick, and some tests might override the tick and accidentally
	 * forget to include the parent function. */
	int32 HitchesDetected = 0;
	
	bool ContainsErrorInSpreadsheet = false;

	virtual void PrepareTest() override;
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

private:
	int32 RowIndex = 0;
	int32 RowOffset = 0;
};
