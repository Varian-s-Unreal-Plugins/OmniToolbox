// Copyright (C) Varian Daemon 2025. All Rights Reserved.
#include "OmniRuntimeMacros.h"

// struct FVoxelConsoleVariable
// {
// 	TFunction<void()> OnChanged;
// 	TFunction<void()> Tick;
// };
// TArray<FVoxelConsoleVariable>& GetVoxelConsoleVariables()
// {
// 	static TArray<FVoxelConsoleVariable> Variables;
// 	return Variables;
// }
//
// FOmniConsoleVariableHelper::FOmniConsoleVariableHelper(
// 	TFunction<void()> OnChanged,
// 	TFunction<void()> Tick)
// {
// 	GetVoxelConsoleVariables().Add(FVoxelConsoleVariable
// 	{
// 		MoveTemp(OnChanged),
// 		MoveTemp(Tick)
// 	});
// }
