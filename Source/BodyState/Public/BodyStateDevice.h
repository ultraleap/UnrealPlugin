

#pragma once

#include "BodyStateDeviceConfig.h"
#include "BodyStateInputInterface.h"

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
	class UBodyStateSkeleton* Skeleton = nullptr;

	IBodyStateInputRawInterface* InputCallbackDelegate;
};
