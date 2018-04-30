// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BodyStateDevice.generated.h"

USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateDevice
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int32 DeviceId;

	UPROPERTY()
	FBodyStateDeviceConfig Config;

	UPROPERTY()
	class UBodyStateSkeleton* Skeleton;

	IBodyStateInputRawInterface* InputCallbackDelegate;
};
