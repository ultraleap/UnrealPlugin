/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "LeapTrackingSettings.h"
#include "LeapBlueprintFunctionLibrary.h"


#if WITH_EDITOR
void ULeapTrackingSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	TArray<FString> Serials;
	ULeapBlueprintFunctionLibrary::SetLeapDeviceHints(Serials, UltraleapHints);
}
#endif
