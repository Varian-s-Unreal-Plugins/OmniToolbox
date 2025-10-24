// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#include "OmniToolboxEditor.h"

#include "OmniToolboxThumbnailRenderer.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintThumbnailRenderer.h"
#include "Config/DS_OmniToolboxSettings.h"
#include "Developer/I_AssetDetails.h"
#include "Interfaces/IPluginManager.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"


#define LOCTEXT_NAMESPACE "FOmniToolboxEditorModule"

void FOmniToolboxEditorModule::StartupModule()
{

}

void FOmniToolboxEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomClassLayout("Object");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOmniToolboxEditorModule, OmniToolboxEditor)