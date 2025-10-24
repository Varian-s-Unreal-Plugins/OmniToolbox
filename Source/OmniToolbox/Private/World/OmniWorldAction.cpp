// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "World/OmniWorldAction.h"

FPrimaryAssetId UOmniWorldAction::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FName("OmniWorldAction"), GetFName());
}
