// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Developer/I_AssetDetails.h"

#include "OmniSpreadsheetObject.generated.h"

USTRUCT(BlueprintType)
struct FOmniSpreadsheetCell
{
	GENERATED_BODY()

	UPROPERTY()
	FString Text;

	UPROPERTY()
	FLinearColor Color = FLinearColor::Transparent;

	FOmniSpreadsheetCell() {}
	FOmniSpreadsheetCell(const FString& InText) : Text(InText) {}
};

/**
 * A spreadsheet object is a helper for managing a singular instance of
 * a XLSX file which can be passed around many objects.
 * This can be combined with a subsystem to allow any gameplay code
 * to easily add to a singular spreadsheet.
 *
 * To start, you call @Initialize and declare your headers.
 * Headers are the columns at the very top and dictate how
 * far a row can get.
 *
 * You then call @AddRowByName and store the index.
 * You then call @GetColumnIndexByHeader and store the index
 * To then modify a cell, you call @EditCell and pass in
 * the stored row and column index to modify the cell.
 */
UCLASS(NotBlueprintable, BlueprintType)
class OMNITOOLBOX_API UOmniSpreadsheetObject : public UObject
{
	GENERATED_BODY()

public:

	/** Initialize a new HTML table file */
	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Initialize(const TArray<FString>& InHeaders, FString CustomDirectory = "", FString CustomFileName = "");

	/** File path to the HTML file this object is managing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni Spreadsheet Object")
	FString FilePath;

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Save();

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	int32 AddRowByName(FString RowName, bool bAutoSave = true);

	/** Finds a column index by its header name. Returns -1 if not found. */
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintPure)
	int32 GetColumnIndexByHeader(FString HeaderName);
	
	/** Edits a specific cell by row/column index. */
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	bool EditCellByColumnName(int32 RowIndex, FString ColumnName, FString NewValue, bool bAutoSave = true);

	/** Edits a specific cell by row/column index. */
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	bool EditCell(int32 RowIndex, int32 ColumnIndex, FString NewValue, bool bAutoSave = true);
	
	/**Sets background color for a cell*/
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	bool ColorCellByColumnName(int32 RowIndex, FString ColumnName, FLinearColor NewColor, bool bAutoSave = true);
	
	/**Sets background color for a cell*/
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	bool ColorCell(int32 RowIndex, int32 ColumnIndex, FLinearColor NewColor, bool bAutoSave = true);
	
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	FOmniSpreadsheetCell GetCellData(int32 Row, int32 Column);
	
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable, BlueprintPure)
	int32 GetNumberOfRows();

	/** Returns the HTML document string. */
	FString SerializeToString() const;

	TArray<FString> Headers;
	TArray<TArray<FOmniSpreadsheetCell>> Rows;
	
	static FString ToHTMLColor(const FLinearColor& C)
	{
		if (C.A <= 0.0f)
		{
			return TEXT(""); // transparent → no style attribute
		}

		FColor SRGB = C.ToFColor(true);
		return FString::Printf(TEXT("#%02X%02X%02X"), SRGB.R, SRGB.G, SRGB.B);
	}
};
