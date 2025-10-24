// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "OmniAssetDefinition.h"

#include "AssetDefinitionRegistry.h"
#include "AssetToolsModule.h"
#include "ClassViewerModule.h"
#include "OmniClassPickerDialogue.h"
#include "Config/DS_OmniToolboxSettings.h"
#include "Developer/I_AssetDetails.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "OmniToolbox/Public/OmniRuntimeMacros.h"

Omni_OnModuleStarted("OmniToolboxEditor")
{
	TArray<UClass*> Classes;
	for(TObjectIterator<UClass> CurrentClass; CurrentClass; ++CurrentClass)
	{
		if(CurrentClass->ImplementsInterface(UI_AssetDetails::StaticClass()))
		{
			if(II_AssetDetails::Execute_AppearsInContextMenu(CurrentClass->GetDefaultObject()))
			{
				Classes.Add(*CurrentClass);
			}
		}
	}

	for(auto& CurrentClass : GetDefault<UDS_OmniToolboxSettings>()->ContextMenuEntries)
	{
		if(CurrentClass.LoadSynchronous()->ImplementsInterface(UI_AssetDetails::StaticClass()))
		{
			if(II_AssetDetails::Execute_AppearsInContextMenu(CurrentClass.LoadSynchronous()->GetDefaultObject()))
			{
				Classes.Add(CurrentClass.LoadSynchronous());
			}
		}
	}

	int32 CurrentIndex = 0;
	for(UClass* Class : Classes)
	{
		const FString ClassName = "/Script/OmniToolboxEditor.OmniFactoryDummy" + FString::FromInt(CurrentIndex++);
		const TSubclassOf<UOmniFactory> FactoryClass = FindObject<UClass>(nullptr, *ClassName);
		if(!FactoryClass)
		{
			return; //Ensure this?
		}
		
		UOmniAssetDefinition* AssetDefinition = NewObject<UOmniAssetDefinition>();

		TArray<FText> Categories;
		AssetDefinition->Class = Class;
		AssetDefinition->AssetDisplayName = Class->GetDisplayNameText();
		AssetDefinition->AssetColor = II_AssetDetails::Execute_GetAssetColor(Class->GetDefaultObject());
		if(AssetDefinition->AssetColor.Equals(FLinearColor()))
		{
			AssetDefinition->AssetColor = FColor::Cyan;
		}
		
		FString PluginName;
		const FString& ModuleDependency = FPackageName::GetShortName(Class->GetPackage()->GetName());
		
		TArray<FModuleStatus> ModuleStatuses;
		const FModuleManager& ModuleManager = FModuleManager::Get();
		ModuleManager.QueryModules(ModuleStatuses);
		for (FModuleStatus& ModuleStatus : ModuleStatuses)
		{
			if (ModuleStatus.bIsLoaded && ModuleStatus.Name == ModuleDependency)
			{
				// this is the module's plugin
				const TSharedPtr<IPlugin>& OwnerPlugin = IPluginManager::Get().GetModuleOwnerPlugin(*ModuleDependency);
				if(OwnerPlugin.IsValid())
				{
					PluginName = OwnerPlugin->GetFriendlyName();
				}
			}
		}

		if(PluginName.IsEmpty() == false)
		{
			Categories.Add(FText::FromString(PluginName));
		}
		
		Categories.Append(II_AssetDetails::Execute_GetAssetsCategories(Class->GetDefaultObject()));

		if(Categories.IsEmpty())
		{
			Categories.Add(FText::FromString("Unknown assets"));
		}

		AssetDefinition->Categories = FAssetCategoryPath(Categories);
		
		UOmniFactory* FactoryCDO = FactoryClass->GetDefaultObject<UOmniFactory>();
		const FString TargetClassName = Class->GetPathName();
		const TSubclassOf<UObject> TargetClass = FindObject<UClass>(nullptr, *TargetClassName);
		
		FactoryCDO->SupportedClass = TargetClass;
		FactoryCDO->bEditAfterNew = true;
		FactoryCDO->UseChildrenDialogueWindow = II_AssetDetails::Execute_UseChildrenCreationWindow(Class->GetDefaultObject());
		
		// The Registry might be null if the module owning the instanced CDO is loaded late into editor lifecycle.
		if (UAssetDefinitionRegistry* AssetDefinitionRegistry = UAssetDefinitionRegistry::Get())
		{
			AssetDefinitionRegistry->RegisterAssetDefinition(AssetDefinition);
		}
	}
}

void UOmniFactory::PostInitProperties()
{
	Super::PostInitProperties();

	/**A new instance is made when we actually click the asset.
	 * Fetch the CDO, which we have modified, and copy the data.
	 * If you add any variables to this factory, you need to update this. */
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		bCreateNew = GetClass()->GetDefaultObject<UOmniFactory>()->bCreateNew;
		SupportedClass = GetClass()->GetDefaultObject<UOmniFactory>()->SupportedClass;
		UseChildrenDialogueWindow = GetClass()->GetDefaultObject<UOmniFactory>()->UseChildrenDialogueWindow;
	}
}

bool UOmniFactory::ShouldShowInNewMenu() const
{
	return IsValid(SupportedClass);
}

FText UOmniFactory::GetDisplayName() const
{
	if(!SupportedClass)
	{
		return FText::FromString("Unsupported Class");
	}
	
	return SupportedClass.Get()->GetDisplayNameText();
}

bool UOmniFactory::ConfigureProperties()
{
	if(!UseChildrenDialogueWindow)
	{
		return Super::ConfigureProperties();
	}

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	const TSharedRef<SOmniClassPickerDialogue> Filter = MakeShared<SOmniClassPickerDialogue>();
	Filter->Class = SupportedClass;
	Filter->ParentClass = SupportedClass;

	return Filter->ConfigureProperties(this);
}

UObject* UOmniFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                        UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	//Handle blueprint creation.
	if(FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass) && !InClass->IsChildOf(UDataAsset::StaticClass()))
	{
		// Create new Blueprint
		UObject* NewObject = 
		FKismetEditorUtilities::CreateBlueprint(
			SupportedClass,
			InParent,
			InName,
			BPTYPE_Normal,
			UBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass(),
			NAME_None
		);

		NewObject->Modify(true);
		return NewObject;
	}
	//Handle data asset creation.
	else if(InClass->IsChildOf(UDataAsset::StaticClass()))
	{
		return NewObject<UObject>(InParent, InClass, InName, Flags);
	}

	return nullptr;
}

FString UOmniFactory::GetToolTipDocumentationPage() const
{
	return "https://varian.gitbook.io/";
}

FString UOmniFactory::GetToolTipDocumentationExcerpt() const
{
	return "varian-docs/inventoryframework/inventory-framework/classes-and-settings/ac_inventory#inventory-extension-components";
}
