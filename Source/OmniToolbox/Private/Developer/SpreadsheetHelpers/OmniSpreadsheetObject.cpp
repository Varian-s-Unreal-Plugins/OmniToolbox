// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Developer/SpreadsheetHelpers/OmniSpreadsheetObject.h"

bool UOmniSpreadsheetObject::Initialize(const TArray<FText>& InHeaders)
{
	for(auto& CurrentHeader : InHeaders)
	{
		Headers.Add(CurrentHeader.ToString());
	}
	
	//Create directory if missing
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("CSV");
	IFileManager::Get().MakeDirectory(*Directory, true);

	// Build timestamped filename
	const FDateTime Now = FDateTime::Now();
	const FString Timestamp = FString::Printf(TEXT("%02d-%02d-%04d_%02d-%02d-%02d"),
		Now.GetDay(), Now.GetMonth(), Now.GetYear(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());

	const FString FileName = FString::Printf(TEXT("CSV_%s.xlsx"), *Timestamp);
	FilePath = Directory / FileName;

	// Save initial CSV with headers
	if (!Save())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create CSV file at %s"), *FilePath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Created new CSV: %s"), *FilePath);
	return true;
}

bool UOmniSpreadsheetObject::Save()
{
	const FString Data = SerializeToString();
	// Force UTF-8 encoding with BOM so Excel handles it properly
	return FFileHelper::SaveStringToFile(Data, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8);
}

int32 UOmniSpreadsheetObject::AddRowByName(FString RowName, bool bAutoSave)
{
	// Create row sized to match headers
	TArray<FString> NewRow;
	NewRow.SetNum(Headers.Num());

	if (Headers.Num() > 0)
	{
		// Set first cell (often "Name" or "ID") to the given row name
		NewRow[0] = RowName;
	}

	Rows.Add(NewRow);
	const int32 NewIndex = Rows.Num() - 1;

	if (bAutoSave)
	{
		Save();
	}

	return NewIndex;
}

int32 UOmniSpreadsheetObject::GetColumnIndexByHeader(FText HeaderName)
{
	for (int32 i = 0; i < Headers.Num(); ++i)
	{
		if (Headers[i].Equals(HeaderName.ToString(), ESearchCase::IgnoreCase))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UOmniSpreadsheetObject::EditCell(int32 RowIndex, int32 ColumnIndex, FString NewValue, bool bAutoSave)
{
	if (!Rows.IsValidIndex(RowIndex) || !Rows[RowIndex].IsValidIndex(ColumnIndex))
	{
		return false;
	}

	Rows[RowIndex][ColumnIndex] = NewValue;
	return bAutoSave ? Save() : true;
}

FString UOmniSpreadsheetObject::SerializeToString() const
{
	TArray<FString> Lines;
	Lines.Add(FString::Join(Headers, TEXT(",")));

	for (const TArray<FString>& Row : Rows)
	{
		Lines.Add(FString::Join(Row, TEXT(",")));
	}

	return FString::Join(Lines, LINE_TERMINATOR);
}
