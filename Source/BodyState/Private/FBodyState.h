// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IBodyState.h"
#include "BodyStateSkeletonStorage.h"
class FBodyStateHMDDevice;

class FBodyState : public IBodyState
{
public:
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsActive();

	virtual bool IsInputReady() override;

	TSharedPtr< class FBodyStateInputDevice > BodyStateInputDevice;
	virtual TSharedPtr< class FBodyStateInputDevice > GetInputDevice() { return BodyStateInputDevice; }

	//Attach/Detach devices
	virtual int32 AttachDevice(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate) override;
	virtual bool DetachDevice(int32 DeviceID) override;

	virtual UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID) override;

	virtual int32 AttachMergingFunctionForSkeleton(TFunction<void(UBodyStateSkeleton*, float)> InFunction, int32 SkeletonId = 0) override;
	virtual bool RemoveMergingFunction(int32 MergingFunctionId) override;

	virtual void AddBoneSceneListener(UBodyStateBoneComponent* Listener) override;
	virtual void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener) override;

private:
	bool bActive = false;
	TSharedPtr<FBodyStateSkeletonStorage> SkeletonStorage;

	//Built-in devices
	TSharedPtr<FBodyStateHMDDevice> BSHMDDevice;
};