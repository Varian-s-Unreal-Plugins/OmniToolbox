// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Developer/I_AssetDetails.h"

#include "OmniRuntimeMacros.h"

#if WITH_EDITOR
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#endif


// Add default functionality here for any II_AssetDetails functions that are not pure virtual.

/**Handle the documentation button*/
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

						TArray<FOmniDocumentationLinkAndText> Links = II_AssetDetails::Execute_GetDocumentationLinks(TargetObject);
						if(Links.IsEmpty())
						{
							/**No links, we can't display anything */
							break;
						}

						if(Links.Num() == 1)
						{
							/**Add the button that when pressed, will open the URL*/
							FString URL = Links[0].URL;
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

						/**Populate the dropdown menu*/
						FMenuBuilder DetailViewOptions(true, nullptr);
						for(auto& CurrentLink : Links)
						{
							FString URL = CurrentLink.URL;
							DetailViewOptions.AddMenuEntry(
							TAttribute<FText>(FText::FromString(CurrentLink.Text)),
							TAttribute<FText>(),
								FSlateIcon(),
								FUIAction(
									FExecuteAction::CreateLambda([URL]
									{
										FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
									}),
									FCanExecuteAction()
								),
								NAME_None,
								EUserInterfaceActionType::Button
							);
						}

						/**Create a combo box and give it the populated dropdown menu*/
						auto DocsCombo = SNew(SComboButton)
						.ContentPadding(FMargin(4, 0))
						.HasDownArrow(false)
						.ForegroundColor(FSlateColor::UseForeground())
						.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButtonWithIcon")
						.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ViewOptions")))
						.ToolTip(SNew(SToolTip).Text((INVTEXT("View documentation for this asset"))))
						.MenuContent()
						[
							DetailViewOptions.MakeWidget()
						]
						.ButtonContent()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(6, 0)
							.HAlign(HAlign_Fill)
							[
								SNew(SImage)
								.DesiredSizeOverride(FVector2D(20, 20))
								.Image(FAppStyle::Get().GetBrush("Icons.OpenInBrowser"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						];

						InSection.AddEntry(FToolMenuEntry::InitWidget(
							"ViewDocs",
							DocsCombo,
							INVTEXT("View Documentation")));

						break;
					}
				}
			}
		}));
	}
#endif
}
