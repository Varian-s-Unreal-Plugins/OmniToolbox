// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Core/FunctionalTestParents/VanguardFunctionalTest.h"

#include "OmniToolboxVanguard.h"
#include "VanguardFunctionalTestSubsystem.h"
#include "FunctionLibraries/OmniEditorLibrary.h"


void AVanguardFunctionalTest::WriteSpecificResult(EVanguardExcelHeader Header, FString ResultText, int32 RowOverride)
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
				StartNewTest(CurrentTestLog);
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
		 * If not, paint them red and create an error */
		if(LastExpectedResult != ResultText)
		{
			/**Results don't match, report the issue.*/
			UOmniEditorLibrary::RaiseScriptError(FString("Result " + ResultText + " does not match expected result " + LastExpectedResult));
			SpreadsheetObject->ColorCell(Row, ColumnIndex, FLinearColor::Red, false);
			SpreadsheetObject->ColorCell(Row, ExpectedResultColumnIndex, FLinearColor::Red, false);
			ContainsErrorInSpreadsheet = true;
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

void AVanguardFunctionalTest::WriteResult(FString ExpectedResult, FString ActualResult, int32 RowOverride)
{
	if(SpreadsheetObject)
	{
		WriteSpecificResult(EVanguardExcelHeader::ExpectedResult, ExpectedResult, RowOverride);
		WriteSpecificResult(EVanguardExcelHeader::ActualResult, ActualResult, RowOverride);
	}
}

void AVanguardFunctionalTest::StartNewTest(FString InTestName)
{
	if(SpreadsheetObject)
	{
		CurrentTestLog = InTestName;
		/**Add a new row and assign the test label as the tester*/
		RowIndex = SpreadsheetObject->AddRowByName(TestLabel, false);
		/**Assign the TestName into the appropriate column*/
		SpreadsheetObject->EditCell(RowIndex, SpreadsheetObject->GetColumnIndexByHeader(VanguardSpreadSheet::TestHeaders::TestName), InTestName, false);
	}
}

void AVanguardFunctionalTest::PrepareTest()
{
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
			GetClass()->GetAuthoredName());
		
		Subsystem->TestBeginning(this);
	}
	
	Super::PrepareTest();
}

void AVanguardFunctionalTest::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	if(UVanguardFunctionalTestSubsystem* Subsystem = GetWorld()->GetSubsystem<UVanguardFunctionalTestSubsystem>())
	{
		if(FVisualLogger::Get().IsRecordingToFile())
		{
			UOmniEditorLibrary::EnableVislogRecordingToFile(false); 
			
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
				IFileManager::Get().Move(*(FPaths::ProjectSavedDir() / Subsystem->FilepathToUse / (FString::Printf(TEXT("%s_Vislog.bvlog"), *GetClass()->GetAuthoredName()))), *LatestFile);
			}
		}
		
		if(SpreadsheetObject)
		{
			/**Check for any empty ExpectedResult rows*/
			int32 ExpectedResultColumn = SpreadsheetObject->GetColumnIndexByHeader(VanguardSpreadSheet::TestHeaders::ExpectedResult);
			for(int32 i = 1; i < SpreadsheetObject->GetNumberOfRows(); i++)
			{
				FOmniSpreadsheetCell Cell = SpreadsheetObject->GetCellData(i, ExpectedResultColumn);
				if(Cell.Text.IsEmpty())
				{
					UOmniEditorLibrary::RaiseScriptError(FString("Expected Result cell is empty in row " + FString::FromInt(i)));
					TestResult = EFunctionalTestResult::Failed;
				}
			}
		
			if(GenerateSpreadsheets)
			{
				SpreadsheetObject->Save();
			}
		
			if(ContainsErrorInSpreadsheet)
			{
				TestResult = EFunctionalTestResult::Failed;
			}
		}
			
		Subsystem->TestEnding(this, TestResult);
	}
	
	Super::FinishTest(TestResult, Message);
}
