// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Developer/I_AssetDetails.h"

#include "OmniRuntimeMacros.h"

#if WITH_EDITOR
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#endif


// Add default functionality here for any II_AssetDetails functions that are not pure virtual.

Omni_OnModuleStarted("OmniToolboxEditor")
{
#if WITH_EDITOR
	UToolMenus* ToolMenus = UToolMenus::Get();
	FName ToolBarName = "AssetEditorToolbar.CommonActions";

	UToolMenu* FoundMenu = ToolMenus->FindMenu(ToolBarName);

	if (!FoundMenu || !FoundMenu->IsRegistered())
	{
		FoundMenu = UToolMenus::Get()->ExtendMenu("AssetEditorToolbar.CommonActions");
		FToolMenuSection& Section = FoundMenu->FindOrAddSection("CommonActions");

		Section.AddDynamicEntry("View Docs", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			UAssetEditorToolkitMenuContext* MenuContext = InSection.FindContext<UAssetEditorToolkitMenuContext>();
			if (MenuContext && MenuContext->Toolkit.IsValid() && MenuContext->Toolkit.Pin()->IsActuallyAnAsset())
			{
				for (const UObject* EditedAsset : *MenuContext->Toolkit.Pin()->GetObjectsCurrentlyBeingEdited())
				{
					if(IsValid(EditedAsset) && EditedAsset->IsAsset())
					{
						/**Figure out the target object to call the interface functions on.
						 * We need to check for a blueprint, because some assets, like a data asset,
						 * aren't "blueprints" and EditedAsset returns a blueprint if the asset
						 * is a blueprint. */
						const UObject* TargetObject = nullptr;
						
						if(const UBlueprint* Blueprint = Cast<UBlueprint>(EditedAsset))
						{
							TargetObject = Blueprint->ParentClass.GetDefaultObject();
							
						}
						else
						{
							TargetObject = EditedAsset;
						}

						if(!TargetObject->Implements<UI_AssetDetails>())
						{
							break;
						}
						
						if(TargetObject == nullptr)
						{
							break;
						}

						FString URL = II_AssetDetails::Execute_GetDocumentationURL(TargetObject);
						if(URL.IsEmpty())
						{
							/**No URL, we can't */
							break;
						}

						/**Add the button that when pressed, will open the URL*/
						InSection.AddMenuEntry(
								"OpenDocumentationURL",
								INVTEXT("Open Documentation URL"),
								INVTEXT("Open the documentation for this asset in your browser"),
								FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.OpenInBrowser", "Icons.OpenInBrowser"),
								FExecuteAction::CreateLambda([URL]()
									{
										FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
									}));
								break;
					}
				}
			}
		}));
	}
#endif
}
