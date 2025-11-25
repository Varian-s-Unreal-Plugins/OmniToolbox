// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "OmniValidationSettings.h"

#include "BlueprintEditorModule.h"

void UOmniValidationSettings::OpenBlueprintAndFocusNode(UBlueprint* Blueprint, UEdGraph* Graph, UEdGraphNode* Node)
{
	if (!Blueprint || !Graph || !Node) return;

	// Open the Blueprint editor (or bring it to front)
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(Blueprint);

	// Get the currently opened asset editor
	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, /*bFocusIfOpen=*/false);
	if (!EditorInstance) return;

	// Get the Blueprint editor interface
	IBlueprintEditor* BlueprintEditor = static_cast<IBlueprintEditor*>(EditorInstance);
	if (!BlueprintEditor) return;

	// Open the graph tab and focus on the graph
	BlueprintEditor->OpenGraphAndBringToFront(Graph);

	// Select and zoom to the node
	BlueprintEditor->JumpToHyperlink(Node, false); // false: don't request rename
}
