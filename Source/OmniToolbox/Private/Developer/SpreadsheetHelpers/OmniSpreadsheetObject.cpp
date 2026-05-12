// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Developer/SpreadsheetHelpers/OmniSpreadsheetObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UOmniSpreadsheetObject::Initialize(const TArray<FString>& InHeaders, bool DoNotCreateFile , FString CustomDirectory, FString CustomFileName)
{
	for(auto& CurrentHeader : InHeaders)
	{
		Headers.Add(CurrentHeader);
	}
	
	if(DoNotCreateFile)
	{
		return;
	}
	
	//Create directory if missing
	const FString Directory = FPaths::ProjectSavedDir() / (CustomDirectory.IsEmpty() ? TEXT("Spreadsheets") : CustomDirectory);
	IFileManager::Get().MakeDirectory(*Directory, true);

	//Timestamped filename
	const FDateTime Now = FDateTime::Now();
	const FString Timestamp = FString::Printf(TEXT("%02d-%02d-%04d_%02d-%02d-%02d"),
		Now.GetDay(), Now.GetMonth(), Now.GetYear(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());

	FileName = CustomFileName.IsEmpty() ? FString::Printf(TEXT("%s.html"), *Timestamp) : FString::Printf(TEXT("%s.html"), *CustomFileName);
	FilePath = Directory / FileName;
}

bool UOmniSpreadsheetObject::Save()
{
	const FString Data = SerializeToString();
	return FFileHelper::SaveStringToFile(Data, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8);
}

int32 UOmniSpreadsheetObject::AddRowByName(FString RowName, bool bAutoSave)
{
	//Create row sized to match headers
	TArray<FOmniSpreadsheetCell> NewRow;
	NewRow.SetNum(Headers.Num());

	if (Headers.Num() > 0)
	{
		//Set first cell (often "Name" or "ID") to the given row name
		NewRow[0].Text = RowName;
	}

	Rows.Add(NewRow);
	const int32 Index = Rows.Num() - 1;

	if (bAutoSave)
	{
		Save();
	}

	return Index;
}

int32 UOmniSpreadsheetObject::GetColumnIndexByHeader(FString HeaderName)
{
	for (int32 i = 0; i < Headers.Num(); ++i)
	{
		if (Headers[i].Equals(HeaderName, ESearchCase::IgnoreCase))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UOmniSpreadsheetObject::EditCellByColumnName(int32 RowIndex, FString ColumnName, FString NewValue, bool bAutoSave)
{
	return EditCell(RowIndex, GetColumnIndexByHeader(ColumnName), NewValue, bAutoSave);
}

bool UOmniSpreadsheetObject::EditCell(int32 RowIndex, int32 ColumnIndex, FString NewValue, bool bAutoSave)
{
	if (!Rows.IsValidIndex(RowIndex) || !Rows[RowIndex].IsValidIndex(ColumnIndex))
	{
		return false;
	}

	Rows[RowIndex][ColumnIndex].Text = NewValue;
	return bAutoSave ? Save() : true;
}

bool UOmniSpreadsheetObject::ColorCellByColumnName(int32 RowIndex, FString ColumnName, FLinearColor NewColor,
	bool bAutoSave)
{
	return ColorCell(RowIndex, GetColumnIndexByHeader(ColumnName), NewColor, bAutoSave);
}

bool UOmniSpreadsheetObject::ColorCell(int32 RowIndex, int32 ColumnIndex, FLinearColor NewColor, bool bAutoSave)
{
	if (!Rows.IsValidIndex(RowIndex) || !Rows[RowIndex].IsValidIndex(ColumnIndex))
	{
		return false;
	}

	Rows[RowIndex][ColumnIndex].Color = NewColor;

	return bAutoSave ? Save() : true;
}

FOmniSpreadsheetCell UOmniSpreadsheetObject::GetCellData(int32 Row, int32 Column)
{
	if(Rows.IsValidIndex(Row) && Rows[Row].IsValidIndex(Column))
	{
		return Rows[Row][Column];
	}
	
	return FOmniSpreadsheetCell();
}

int32 UOmniSpreadsheetObject::GetNumberOfRows()
{
	return Rows.Num();
}

FString UOmniSpreadsheetObject::SerializeToString() const
{
	FString Html;

	Html += TEXT("<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
	Html += TEXT("<style>\n");
	Html += TEXT("table { border-collapse: collapse; font-family: sans-serif; }\n");
	Html += TEXT("th, td { border: 1px solid #ccc; padding: 4px 8px; }\n");
	Html += TEXT("th { background-color: #f0f0f0; font-weight: bold; }\n");
	Html += TEXT("</style>\n</head>\n<body>\n<table>\n");

	// Headers
	Html += TEXT("<tr>");
	for (const FString& H : Headers)
	{
		Html += TEXT("<th>") + H + TEXT("</th>");
	}
	Html += TEXT("</tr>\n");

	// Rows
	for (const TArray<FOmniSpreadsheetCell>& Row : Rows)
	{
		Html += TEXT("<tr>");
		for (const FOmniSpreadsheetCell& Cell : Row)
		{
			FString Style;

			const FString ColorHex = ToHTMLColor(Cell.Color);
			if (!ColorHex.IsEmpty())
			{
				Style = TEXT(" style=\"background-color: ") + ColorHex + TEXT(";\"");
			}

			Html += TEXT("<td") + Style + TEXT(">") + Cell.Text + TEXT("</td>");
		}
		Html += TEXT("</tr>\n");
	}

	Html += TEXT("</table>\n</body>\n</html>");

	return Html;
}
