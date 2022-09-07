/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "IUltraleapTrackingPlugin.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class FUltraleapTrackingPlugin : public IUltraleapTrackingPlugin
{
public:
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void AddEventDelegate(const ULeapComponent* EventDelegate) override;
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate) override;
	virtual FLeapStats GetLeapStats(const FString& DeviceSerial) override;
	virtual void SetOptions(const FLeapOptions& Options, const TArray<FString>& DeviceSerials) override;
	virtual FLeapOptions GetOptions(const FString& DeviceSerial) override;
	virtual void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible, const FString& DeviceSerial) override;
	virtual void GetLatestFrameData(FLeapFrameData& OutData,const FString& DeviceSerial) override;
	virtual void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials) override;
	virtual void GetAttachedDevices(TArray<FString>& Devices) override;

	virtual void ShutdownLeap() override;
	virtual void SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ,
		ELeapQuatSwizzleAxisB ToW, const TArray<FString>& DeviceSerials) override;

	virtual ILeapConnector* GetConnector() override;

	bool IsActive();

private:
	TSharedPtr<class FUltraleapTrackingInputDevice> LeapInputDevice;
	TArray<ULeapComponent*> DeferredComponentList;

	bool bActive = false;
	void* LeapDLLHandle;

	void* GetLeapHandle();
};
