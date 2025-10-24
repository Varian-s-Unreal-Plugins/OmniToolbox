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
 */
UCLASS(NotBlueprintable, BlueprintType)
class OMNITOOLBOX_API UOmniSpreadsheetObject : public UObject
{
	GENERATED_BODY()

public:

	/** Initialize a new CSV file (creates or overwrites existing file). */
	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Initialize(const TArray<FText>& InHeaders);

	/** File path this CSV is managing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni Spreadsheet Object")
	FString FilePath;

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	bool Save();

	UFUNCTION(BlueprintCallable, Category = "Omni Spreadsheet Object")
	int32 AddRowByName(FString RowName, bool bAutoSave = true);

	/** Finds a column index by its header name. Returns -1 if not found. */
	UFUNCTION(BlueprintPure, Category = "Omni Spreadsheet Object")
	int32 GetColumnIndexByHeader(FText HeaderName);

	/** Edits a specific cell by row/column index. */
	UFUNCTION(BlueprintPure, Category = "Omni Spreadsheet Object")
	bool EditCell(int32 RowIndex, int32 ColumnIndex, FString NewValue, bool bAutoSave = true);

	FString SerializeToString() const;
	
	TArray<FString> Headers;
	
	TArray<TArray<FString>> Rows;
};
