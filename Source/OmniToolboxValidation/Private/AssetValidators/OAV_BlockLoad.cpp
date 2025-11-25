// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "AssetValidators/OAV_BlockLoad.h"

#include "K2Node_CallFunction.h"
#include "OmniValidationSettings.h"
#include "Misc/DataValidation.h"

bool UOAV_BlockLoad::OmniValidatorCanValidateAsset(const FAssetData& InAssetData, UObject* InObject,
	FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UBlueprint>();
}

EDataValidationResult UOAV_BlockLoad::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(InAsset);
	if (!Blueprint) return EDataValidationResult::NotValidated;

	EDataValidationResult DataValidationResult = EDataValidationResult::Valid;

	TArray<UEdGraph*> AllGraphs;
	AllGraphs.Append(Blueprint->FunctionGraphs);
	AllGraphs.Append(Blueprint->UbergraphPages);
	
	for (UEdGraph* Graph : AllGraphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (IsBlockingLoad(Node))
			{
				// Create a tokenized message with an action to open the Blueprint and focus the node
				TSharedRef<FTokenizedMessage> TokenizedMessage = FTokenizedMessage::Create((EMessageSeverity::Error), FText::FromString(TEXT("Blocking (synchronous) loading nodes found.")));

				TokenizedMessage->AddToken(FActionToken::Create(
					FText::FromString(TEXT("Open Blueprint and Focus Node")),
					FText::FromString(TEXT("Open Blueprint and Focus Node")),
					FOnActionTokenExecuted::CreateLambda([Blueprint, Graph, Node]()
						{
							UOmniValidationSettings::OpenBlueprintAndFocusNode(Blueprint, Graph, Node);
						}),
					false
				));

				Context.AddMessage(TokenizedMessage);

				DataValidationResult = EDataValidationResult::Invalid;
			}
		}
	}

	return DataValidationResult;
}

bool UOAV_BlockLoad::IsBlockingLoad(UEdGraphNode* Node)
{
	UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node);

	if (!CallFunctionNode)
	{
		// Not a function call node
		return false;
	}

	static const FName LoadAssetBlockingFunctionName(TEXT("LoadAsset_Blocking"));
	static const FName LoadClassAssetBlockingFunctionName(TEXT("LoadClassAsset_Blocking"));
	FName FunctionName = CallFunctionNode->GetFunctionName();

	if (FunctionName == LoadAssetBlockingFunctionName)
	{
		return true;
	}

	if (FunctionName == LoadClassAssetBlockingFunctionName)
	{
		return true;
	}


	// Not a blocking (synchronous) loading function
	return false;
}
