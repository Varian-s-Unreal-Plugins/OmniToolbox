// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Developer/I_AssetDetails.h"

#include "OmniSpreadsheetObject.generated.h"

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

	/** Initialize a new XLSX file */
	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Initialize(const TArray<FText>& InHeaders);

	/** File path to the XLSX this object is managing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni Spreadsheet Object")
	FString FilePath;

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Save();

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	int32 AddRowByName(FString RowName, bool bAutoSave = true);

	/** Finds a column index by its header name. Returns -1 if not found. */
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintPure)
	int32 GetColumnIndexByHeader(FText HeaderName);

	/** Edits a specific cell by row/column index. */
	UFUNCTION(Category = "Omni Spreadsheet Object", BlueprintCallable)
	bool EditCell(int32 RowIndex, int32 ColumnIndex, FString NewValue, bool bAutoSave = true);

	FString SerializeToString() const;
	
	TArray<FString> Headers;
	
	TArray<TArray<FString>> Rows;
};
