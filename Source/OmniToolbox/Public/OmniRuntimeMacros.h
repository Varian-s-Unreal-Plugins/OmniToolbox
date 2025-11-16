// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "Subsystems/OmniEngineSubsystem.h"
#include "OmniToolbox.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"

/**Trace with string label*/
#define Omni_InsightsTrace_Str(Text) TRACE_CPUPROFILER_EVENT_SCOPE_STR(Text)
#define Omni_InsightsTrace() TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__)


struct FOmniRunOnStartupHelper
{
	FOmniRunOnStartupHelper(FName InModuleName, TFunction<void()> InLambda)
	{
		FModuleManager::Get().OnModulesChanged().AddLambda(
			[InModuleName, InLambda](FName ModuleName, EModuleChangeReason Reason)
			{
				if (Reason == EModuleChangeReason::ModuleLoaded && ModuleName == InModuleName)
				{
					InLambda();
				}
			}
		);

		//If the module is already loaded (editor, hot reload, etc.), run immediately
		if (FModuleManager::Get().IsModuleLoaded(InModuleName))
		{
			InLambda();
		}
	}
};

struct FOmniRunOnPostEngineInitHelper
{
	FOmniRunOnPostEngineInitHelper(TFunction<void()> InLambda)
	{
		FCoreDelegates::OnPostEngineInit.AddLambda(
			[InLambda]()
			{
				InLambda();
			}
		);

		//TODO: Find out if we can execute the lambda if the editor is already started, but I don't see how that would happen
	}
};

template<int32 N>
struct TOmniCounterDummy { TOmniCounterDummy() = default; };

#define Omni_OnPostEngineInit() \
static void OmniPostEngineInitFunction(TOmniCounterDummy<__COUNTER__>); \
static const FOmniRunOnPostEngineInitHelper PREPROCESSOR_JOIN(FOmniRunOnPostEngineInitHelper, __COUNTER__)([] \
{ \
OmniPostEngineInitFunction(TOmniCounterDummy<__COUNTER__ - 2>()); \
}); \
static void OmniPostEngineInitFunction(TOmniCounterDummy<__COUNTER__ - 3>)

#define Omni_OnModuleStarted(ModuleName) \
static void OmniStartupFunction(TOmniCounterDummy<__COUNTER__>); \
static const FOmniRunOnStartupHelper PREPROCESSOR_JOIN(OmniRunOnStartupHelper, __COUNTER__)(ModuleName, [] \
{ \
	OmniStartupFunction(TOmniCounterDummy<__COUNTER__ - 2>()); \
}); \
static void OmniStartupFunction(TOmniCounterDummy<__COUNTER__ - 3>)

#define Omni_SetClassIcon(PluginName, ClassName, SvgName) \
static void PREPROCESSOR_JOIN(OmniSetClassIcon_Init_, __COUNTER__)(); \
static const FOmniRunOnStartupHelper PREPROCESSOR_JOIN(OmniSetClassIcon_Helper_, __COUNTER__)(TEXT(#PluginName), [] \
{ \
	FCoreDelegates::OnPostEngineInit.AddLambda([]() \
	{ \
		if (GEngine && GEngine->IsInitialized()) \
		{ \
			if (UOmniEngineSubsystem* OmniStyle = GEngine->GetEngineSubsystem<UOmniEngineSubsystem>()) \
			{ \
			OmniStyle->RegisterClassIcon(TEXT(#PluginName), TEXT(#ClassName), TEXT(#SvgName)); \
			} \
		} \
	}); \
}); \
static void PREPROCESSOR_JOIN(OmniSetClassIcon_Init_, __COUNTER__)();

// #define Omni_SetClassIcon(PluginName, ClassName, SvgName)                              \
// 	/* freeze the counter so all uses match */                                         \
// 	UE_DISABLE_OPTIMIZATION                                                            \
// 	constexpr int32 OmniSetClassIcon_Counter = __COUNTER__;                            \
// 	static void PREPROCESSOR_JOIN(OmniSetClassIcon_Init_, OmniSetClassIcon_Counter)(); \
// 	static const FOmniRunOnStartupHelper PREPROCESSOR_JOIN(OmniSetClassIcon_Helper_, OmniSetClassIcon_Counter)(TEXT(#PluginName), [] \
// 	{                                                                                  \
// 		FCoreDelegates::OnPostEngineInit.AddLambda([]()                                \
// 		{                                                                              \
// 			if (GEngine && GEngine->IsInitialized())                                   \
// 			{                                                                          \
// 				if (UOmniEngineSubsystem* OmniStyle = GEngine->GetEngineSubsystem<UOmniEngineSubsystem>()) \
// 				{                                                                      \
// 					OmniStyle->RegisterClassIcon(TEXT(#PluginName), TEXT(#ClassName), TEXT(#SvgName)); \
// 				}                                                                      \
// 			}                                                                          \
// 		});                                                                            \
// 	});                                                                                \
// 	static void PREPROCESSOR_JOIN(OmniSetClassIcon_Init_, OmniSetClassIcon_Counter)(); \
// 	UE_ENABLE_OPTIMIZATION

#define Omni_PlacementPaletteCategory(PluginName, NameToDisplay, SVGName, SortOrder)     \
Omni_OnModuleStarted("PlacementMode")                                                    \
{\
if (UOmniEngineSubsystem* OmniStyle = GEngine->GetEngineSubsystem<UOmniEngineSubsystem>()) \
{ \
OmniStyle->RegisterPlacementPaletteCategory(PluginName, NameToDisplay, SVGName, SortOrder); \
} \
}

#define Omni_AddActorToPlacementPaletteCategory(Category, Class) \
Omni_OnPostEngineInit() \
{ \
	if (UOmniEngineSubsystem* OmniStyle = GEngine->GetEngineSubsystem<UOmniEngineSubsystem>()) \
	{ \
		OmniStyle->AddClassToPlacementPalette(Category, Class); \
	} \
}
