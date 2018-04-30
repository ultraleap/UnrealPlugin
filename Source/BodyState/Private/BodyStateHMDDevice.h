#pragma once

#include "BodyStatePrivatePCH.h"
#include "BodyStateDeviceConfig.h"


class FBodyStateHMDDevice : public IBodyStateInputRawInterface
{
public:
	FBodyStateHMDDevice();
	virtual ~FBodyStateHMDDevice();
	int32 HMDDeviceIndex;
	FBodyStateDeviceConfig Config;

	virtual void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton) override;
	virtual void OnDeviceDetach() override;
};