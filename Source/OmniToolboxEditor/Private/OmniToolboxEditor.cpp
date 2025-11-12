// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#include "OmniToolboxEditor.h"

#include "AssetDefinitionRegistry.h"
#include "OmniToolboxThumbnailRenderer.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "OmniAssetDefinition.h"
#include "WidgetBlueprint.h"
#include "Config/DS_OmniToolboxSettings.h"
#include "Customizations/FloatProvidersCustomization.h"


#define LOCTEXT_NAMESPACE "FOmniToolboxEditorModule"

void FOmniToolboxEditorModule::StartupModule()
{

}

void FOmniToolboxEditorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOmniToolboxEditorModule, OmniToolboxEditor)