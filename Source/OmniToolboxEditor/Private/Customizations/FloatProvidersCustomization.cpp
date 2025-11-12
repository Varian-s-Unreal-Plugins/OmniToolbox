// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "Customizations/FloatProvidersCustomization.h"

#include "DetailLayoutBuilder.h"
#include "FloatProvider.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "InstancedStructDetails.h"
#include "IPropertyUtilities.h"
#include "OmniRuntimeMacros.h"
#include "PropertyBindingExtension.h"
#include "SInstancedStructPicker.h"
#include "StructViewerModule.h"
#include "Widgets/Input/SSpinBox.h"

Omni_OnModuleStarted(UE_MODULE_NAME)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(FOmniFloatProvider::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFloatProvidersCustomization::MakeInstance));
}

class FFloatProviderInstanceWrapperDataDetails : public FInstancedStructDataDetails
{
public:
	FFloatProviderInstanceWrapperDataDetails(TSharedPtr<IPropertyHandle> InStructProperty)
		: FInstancedStructDataDetails(InStructProperty)
	{
	}

	virtual void OnChildRowAdded(IDetailPropertyRow& ChildRow) override
	{
		if (ID.IsValid())
		{
			TSharedPtr<IPropertyHandle> ChildPropHandle = ChildRow.GetPropertyHandle();
			check(ChildPropHandle.IsValid());
			
			// Pass the node ID to binding extension. Since the properties are added using AddChildStructure(), we break the hierarchy and cannot access parent.
			ChildPropHandle->SetInstanceMetaData(UE::PropertyBinding::MetaDataStructIDName, LexToString(ID));
		}
	}

private:
	FGuid ID;
};

void FFloatProvidersCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructProperty = StructPropertyHandle;
	PropUtils = StructCustomizationUtils.GetPropertyUtilities();
	FloatProviderPropertyHandle = StructProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOmniFloatProvider, FloatProvider));

	TArray<void*> StructPtrs;
	StructPropertyHandle->AccessRawData(StructPtrs);

	
	
	if(StructPtrs.IsValidIndex(0))
	{
		const int32 ArrayIndex = FloatProviderPropertyHandle->IsValidHandle() ? FloatProviderPropertyHandle->GetIndexInArray() : INDEX_NONE;
		FOmniFloatProvider* OmniFloatProviderPtr = reinterpret_cast<FOmniFloatProvider*>(StructPtrs[ArrayIndex == INDEX_NONE ? 0 : ArrayIndex]);
		if(OmniFloatProviderPtr)
		{
			if(OmniFloatProviderPtr->FloatProvider.IsValid() == false)
			{
				/**We never want a scenario where a float provider is null,
				 * always resort to a basic float provider is none is provided */
				OmniFloatProviderPtr->FloatProvider.InitializeAs<FBasicFloatProvider>();
			}
			
			if(OmniFloatProviderPtr->FloatProvider.IsValid())
			{
				MutableProvider = OmniFloatProviderPtr->FloatProvider.GetMutablePtr<>();
				Value = OmniFloatProviderPtr->FloatProvider.GetMutablePtr<>()->GetFloat();
				IsBasicFloat = OmniFloatProviderPtr->FloatProvider.GetScriptStruct() == FBasicFloatProvider::StaticStruct();
			}
		}
	}

	/**V: I know this is a lot of copy-pasted code, but I am too lazy to do this properly*/
	if(StructPropertyHandle->GetParentHandle().IsValid() && StructPropertyHandle->GetParentHandle()->AsMap().IsValid())
	{
		/**This is a TMap property, we can't use WholeRowContent without breaking it*/
		HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()]
		.ValueContent()[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(ComboButton, SComboButton)
				.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
				.OnGetMenuContent(this, &FFloatProvidersCustomization::GenerateStructPicker)
				.ContentPadding(0.f)
				
				.ButtonContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign( VAlign_Center )
					[
						SNew(SSpinBox<float>)
						.Visibility_Lambda([this]{ return IsBasicFloat ? EVisibility::Visible : EVisibility::Collapsed;})
						.Value_Lambda([this]{ return Value; })
						.Delta(0.1)
						.OnValueChanged_Lambda([this](float InValue)
						{
							UE_LOG(LogTemp, Warning, TEXT("Value changed to %f"), InValue);
							if(MutableProvider)
							{
								if(FBasicFloatProvider* BasicFloatProvider = static_cast<FBasicFloatProvider*>(MutableProvider))
								{
									BasicFloatProvider->FloatValue = InValue;
									FloatProviderPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
									Value = InValue;
								}
							}
						})
						.OnValueCommitted_Lambda([this](float InValue, ETextCommit::Type CommitType)
						{
							UE_LOG(LogTemp, Warning, TEXT("Value changed to %f"), InValue);
							if(MutableProvider)
							{
								if(FBasicFloatProvider* BasicFloatProvider = static_cast<FBasicFloatProvider*>(MutableProvider))
								{
									BasicFloatProvider->FloatValue = InValue;
									FloatProviderPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
									Value = InValue;
								}
							}
						})
						
					]
					+ SHorizontalBox::Slot()
					.Padding(4, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &FFloatProvidersCustomization::GetDefinitionDataName)
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					
				]
			]
		];
	}
	else
	{
		HeaderRow.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(ComboButton, SComboButton)
				.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
				.OnGetMenuContent(this, &FFloatProvidersCustomization::GenerateStructPicker)
				.ContentPadding(0.f)
				
				.ButtonContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign( VAlign_Center )
					[
						SNew(SSpinBox<float>)
						.Visibility_Lambda([this]{ return IsBasicFloat ? EVisibility::Visible : EVisibility::Collapsed;})
						.Value_Lambda([this]{ return Value; })
						.Delta(0.1)
						.OnValueChanged_Lambda([this](float InValue)
						{
							UE_LOG(LogTemp, Warning, TEXT("Value changed to %f"), InValue);
							if(MutableProvider)
							{
								if(FBasicFloatProvider* BasicFloatProvider = static_cast<FBasicFloatProvider*>(MutableProvider))
								{
									BasicFloatProvider->FloatValue = InValue;
									FloatProviderPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
									Value = InValue;
								}
							}
						})
						.OnValueCommitted_Lambda([this](float InValue, ETextCommit::Type CommitType)
						{
							UE_LOG(LogTemp, Warning, TEXT("Value changed to %f"), InValue);
							if(MutableProvider)
							{
								if(FBasicFloatProvider* BasicFloatProvider = static_cast<FBasicFloatProvider*>(MutableProvider))
								{
									BasicFloatProvider->FloatValue = InValue;
									FloatProviderPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
									Value = InValue;
								}
							}
						})
						
					]
					+ SHorizontalBox::Slot()
					.Padding(4, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &FFloatProvidersCustomization::GetDefinitionDataName)
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					
				]
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				StructProperty->CreateDefaultPropertyButtonWidgets()
			]
		];
	}
}

void FFloatProvidersCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	check(FloatProviderPropertyHandle);

	if(IsBasicFloat)
	{
		return;
	}
	
	// Add instance directly as child.
	const TSharedRef<FFloatProviderInstanceWrapperDataDetails> DataDetails = MakeShared<FFloatProviderInstanceWrapperDataDetails>(FloatProviderPropertyHandle);
	StructBuilder.AddCustomBuilder(DataDetails);
}

TSharedRef<SWidget> FFloatProvidersCustomization::GenerateStructPicker()
{
	static const FName NAME_HideViewOptions = "HideViewOptions";
	static const FName NAME_ShowTreeView = "ShowTreeView";
	static const FName NAME_DisallowedStructs = "DisallowedStructs";
	
	const bool bHideViewOptions = FloatProviderPropertyHandle->HasMetaData(NAME_HideViewOptions);
	const bool bShowTreeView = FloatProviderPropertyHandle->HasMetaData(NAME_ShowTreeView);

	TSharedRef<FInstancedStructFilter> StructFilter = MakeShared<FInstancedStructFilter>();
	StructFilter->BaseStruct = TBaseStructure<FFloatProviderData>::Get();
	StructFilter->bAllowBaseStruct = false;

	for (TSharedPtr<IPropertyHandle> Handle = FloatProviderPropertyHandle; Handle.IsValid(); Handle = Handle->GetParentHandle())
	{
		const FString& DisallowedStructs = Handle->GetMetaData(NAME_DisallowedStructs);
		if (!DisallowedStructs.IsEmpty())
		{
			TArray<FString> DisallowedStructNames;
			DisallowedStructs.ParseIntoArray(DisallowedStructNames, TEXT(","));

			for (const FString& DisallowedStructName : DisallowedStructNames)
			{
				UScriptStruct* ScriptStruct = FindObject<UScriptStruct>(nullptr, *DisallowedStructName, EFindObjectFlags::None);
				if (ScriptStruct == nullptr)
				{
					ScriptStruct = LoadObject<UScriptStruct>(nullptr, *DisallowedStructName);
				}
				if (ScriptStruct)
				{
					StructFilter->DisallowedStructs.Add(ScriptStruct);
				}
			}
			break;
		}
	}
	

	FStructViewerInitializationOptions Options;
	Options.bShowNoneOption = false;
	Options.StructFilter = StructFilter;
	Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
	Options.DisplayMode = bShowTreeView ? EStructViewerDisplayMode::TreeView : EStructViewerDisplayMode::ListView;
	Options.bAllowViewOptions = !bHideViewOptions;

	FOnStructPicked OnPicked(FOnStructPicked::CreateSP(this, &FFloatProvidersCustomization::OnStructPicked));

	return SNew(SBox)
		.WidthOverride(280.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500.f)
			[
				FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer").CreateStructViewer(Options, OnPicked)
			]
		];
}

void FFloatProvidersCustomization::OnStructPicked(const UScriptStruct* InStruct)
{
	if (FloatProviderPropertyHandle && FloatProviderPropertyHandle->IsValidHandle())
	{
		FScopedTransaction Transaction(INVTEXT("Set Struct"));

		FloatProviderPropertyHandle->NotifyPreChange();

		StructProperty->EnumerateRawData([InStruct](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
		{
			if (FInstancedStruct* InstancedStruct = static_cast<FInstancedStruct*>(RawData))
			{
				InstancedStruct->InitializeAs(InStruct);
			}
			return true;
		});

		FloatProviderPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		FloatProviderPropertyHandle->NotifyFinishedChangingProperties();

		// Property tree will be invalid after changing the struct type, force update.
		if (PropUtils.IsValid())
		{
			PropUtils->ForceRefresh();
		}
	}

	ComboButton->SetIsOpen(false);
}

FText FFloatProvidersCustomization::GetDefinitionDataName() const
{
	check(StructProperty);
	// Note: We pick the first struct, we assume that multi-selection is not used.
	const UScriptStruct* Struct = nullptr;
	StructProperty->EnumerateConstRawData([&Struct](const void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
	{
		if (RawData)
		{
			Struct = static_cast<const FOmniFloatProvider*>(RawData)->FloatProvider.GetScriptStruct();
			return false; // stop
		}
		return true;
	});

	if (Struct)
	{
		return Struct->GetDisplayNameText();
	}
	return INVTEXT("None");
}
