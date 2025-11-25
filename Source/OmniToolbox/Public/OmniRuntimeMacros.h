// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "Subsystems/OmniEngineSubsystem.h"
#include "OmniToolbox.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"

/**Trace with string label*/
#define Omni_InsightsTrace_Str(Text) TRACE_CPUPROFILER_EVENT_SCOPE_STR(Text)
#define Omni_InsightsTrace() TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__)
#define Omni_InsightsTrace_Append(Text) TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("%hs ::Text"), __FUNCTION__)


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

/**The console variable is nearly entirely copied from the open-source VoxelCore plugin.
 * Full credit goes to them. I would have never figured out how to handle this by myself. */

FORCEINLINE void OmniConsoleVariable_CallOnChanged() {}
FORCEINLINE void OmniConsoleVariable_CallOnChanged(TFunction<void()> OnChanged) { OnChanged(); }
FORCEINLINE void OmniConsoleVariable_CallOnChanged(TFunction<void()> OnChanged, TFunction<void()> Tick) { OnChanged(); }

FORCEINLINE void OmniConsoleVariable_CallTick() {}
FORCEINLINE void OmniConsoleVariable_CallTick(TFunction<void()> OnChanged) {}
FORCEINLINE void OmniConsoleVariable_CallTick(TFunction<void()> OnChanged, TFunction<void()> Tick) { Tick(); }

struct FOmniConsoleVariableHelper
{
	FOmniConsoleVariableHelper(
		TFunction<void()> OnChanged,
		TFunction<void()> Tick)
	{}
};

#define Omni_ConsoleVariable(Api, Type, Name, Default, Command, Description, ...) \
	Api Type Name = Default; \
	static FAutoConsoleVariableRef CVar_ ## Name( \
		TEXT(Command),  \
		Name,  \
		TEXT(Description)); \
	\
	static const FOmniConsoleVariableHelper PREPROCESSOR_JOIN(OmniConsoleVariableHelper, __COUNTER__)([] \
	{ \
		static Type LastValue = Default; \
		if (LastValue != Name) \
		{ \
			LastValue = Name; \
			OmniConsoleVariable_CallOnChanged(__VA_ARGS__); \
		} \
	}, \
	[] \
	{ \
		OmniConsoleVariable_CallTick(__VA_ARGS__); \
	});
//
// #define Omni_CONSOLE_COMMAND(Command, Description) \
// 	static void OmniConsoleCommand(TOmniCounterDummy<__COUNTER__>, const TArray<FString>& Args); \
// 	static FAutoConsoleCommand PREPROCESSOR_JOIN(OmniAutoCmd, __COUNTER__)( \
// 	    TEXT(Command), \
// 	    TEXT(Description), \
// 		MakeLambdaDelegate([](const TArray<FString>& Args) \
// 		{ \
// 			static_assert(sizeof(Description) > 1, "Missing description"); \
// 			OmniConsoleCommand(TOmniCounterDummy<__COUNTER__ - 2>(), Args); \
// 		})); \
// 	\
// 	static void OmniConsoleCommand(TOmniCounterDummy<__COUNTER__ - 3>, const TArray<FString>& Args)
//
// #define Omni_CONSOLE_WORLD_COMMAND(Command, Description) \
// 	static void OmniConsoleCommand(TOmniCounterDummy<__COUNTER__>, const TArray<FString>& Args, UWorld* World); \
// 	static FAutoConsoleCommand PREPROCESSOR_JOIN(OmniAutoCmd, __COUNTER__)( \
// 	    TEXT(Command), \
// 	    TEXT(Description), \
// 		MakeLambdaDelegate([](const TArray<FString>& Args, UWorld* World, FOutputDevice&) \
// 		{ \
// 			OmniConsoleCommand(TOmniCounterDummy<__COUNTER__ - 2>(), Args, World); \
// 		})); \
// 	\
// 	static void OmniConsoleCommand(TOmniCounterDummy<__COUNTER__ - 3>, const TArray<FString>& Args, UWorld* World)