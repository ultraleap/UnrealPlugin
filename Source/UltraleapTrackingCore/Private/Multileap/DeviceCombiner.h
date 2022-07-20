/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "Async/Async.h"
#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"
#include "LeapC.h"
#include "UltraleapTrackingData.h"
#include "LeapWrapper.h"
#include "FUltraleapDevice.h"

/** Wraps/Abstracts combined devices */
class FDeviceCombiner : public FLeapWrapperBase
{
public:
	LEAP_IMAGE_FRAME_DESCRIPTION* ImageDescription = NULL;
	void* ImageBuffer = NULL;

	FDeviceCombiner(const LEAP_CONNECTION ConnectionHandle, IHandTrackingWrapper* ConnectorIn,
		const TArray<IHandTrackingWrapper*>& DevicesToCombine, const ELeapDeviceCombinerClass DeviceCombinerClass);
	virtual ~FDeviceCombiner();

	// Function Calls for plugin. Mainly uses Open/Close Connection.

	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(
		LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode) override;
	/** Set the LeapWrapperCallbackInterface delegate. Note that only one can be set at any time (static) */
	void SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate) override;

	/** Close the connection, it will nullify the callback delegate */
	virtual void CloseConnection() override;

	virtual void SetPolicy(int64 Flags, int64 ClearFlags) override;
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) override;
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) override;
	// Polling functions

	/** Get latest frame - critical section locked */
	virtual LEAP_TRACKING_EVENT* GetFrame() override;

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) override;

	virtual LEAP_DEVICE_INFO* GetDeviceProperties() override;	 // Used in polling example

	virtual void EnableImageStream(bool bEnable) override;
	virtual int64_t GetNow() override
	{
		return LeapGetNow();
	}
	virtual uint32_t GetDeviceID() override
	{ 
		return DeviceID; 
	}
	virtual FString GetDeviceSerial() override
	{
		FString Ret = CombinedDeviceSerial;
		
		return Ret;
	}

	virtual IHandTrackingDevice* GetDevice() override
	{
		return Device.Get();
	}
	virtual bool MatchDevices(const TArray<FString> DeviceSerials, const ELeapDeviceCombinerClass CombinerClass) override;
	virtual bool ContainsDevice(IHandTrackingWrapper* DeviceWrapper) override;

private:
	void Millisleep(int Milliseconds);

	// Frame and handle data
	uint32_t DeviceID;

	
	LEAP_CONNECTION ConnectionHandle = nullptr;
	// Threading variables
	FCriticalSection* DataLock;
	TFuture<void> ProducerLambdaFuture;

	void CleanupLastDevice();

	// Received LeapC callbacks (filtered by device) converted into game thread events
	// ITrackingDeviceWrapper
	virtual void HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent) override;
	virtual void HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent) override;
	virtual void HandleLogEvent(const LEAP_LOG_EVENT* LogEvent) override;
	virtual void HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent) override;
	virtual void HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingEvent) override;
	virtual void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent) override;
	virtual void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent) override;

	FThreadSafeBool bIsRunning;

	// TaskGraph event references are only stored to help with threading debug for now.
	FGraphEventRef TaskRefConnection;
	FGraphEventRef TaskRefConnectionLost;
	FGraphEventRef TaskRefDeviceFound;
	FGraphEventRef TaskRefDeviceLost;
	FGraphEventRef TaskRefDeviceFailure;
	FGraphEventRef TaskRefTracking;
	FGraphEventRef TaskRefImageComplete;
	FGraphEventRef TaskRefImageError;
	FGraphEventRef TaskRefLog;
	FGraphEventRef TaskRefPolicy;
	FGraphEventRef TaskRefConfigChange;
	FGraphEventRef TaskRefConfigResponse;

	IHandTrackingWrapper* Connector;

	// manages per device functionality in the same way as InputDevice used to
	// in this case this will be a device combiner
	TSharedPtr<FUltraleapDevice> Device;

	TArray<IHandTrackingWrapper*> DevicesToCombine;

	FString CombinedDeviceSerial;
	ELeapDeviceCombinerClass DeviceCombinerClass;

};
