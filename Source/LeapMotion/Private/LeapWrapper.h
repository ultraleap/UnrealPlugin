// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "LeapC.h"
#include "CoreMinimal.h"
#include "Async/Async.h"
#include "HAL/ThreadSafeBool.h"

/** Interface for the passed callback delegate receiving game thread LeapC callbacks */
class LeapWrapperCallbackInterface
{
public:
	virtual void OnConnect() {};
	virtual void OnConnectionLost() {};
	virtual void OnDeviceFound(const LEAP_DEVICE_INFO *Device) {};
	virtual void OnDeviceLost(const char* Serial) {};
	virtual void OnDeviceFailure(
		const eLeapDeviceStatus FailureCode,
		const LEAP_DEVICE FailedDevice) {};
	virtual void OnPolicy(const uint32_t CurrentPolicies) {};
	virtual void OnFrame(const LEAP_TRACKING_EVENT *TrackingEvent) {};
	virtual void OnImage(const LEAP_IMAGE_EVENT *ImageEvent) {};
	virtual void OnLog(
		const eLeapLogSeverity Severity,
		const int64_t Timestamp,
		const char* Message) {};
	virtual void OnConfigChange(const uint32_t RequestID, const bool Success) {};
	virtual void OnConfigResponse(const uint32_t RequestID, LEAP_VARIANT Value) {};
};

/** Wraps LeapC API into a threaded and event driven delegate callback format */
class FLeapWrapper
{
public:
	//LeapC Vars
	FThreadSafeBool bIsRunning;
	FThreadSafeBool bHasFinished;
	bool bIsConnected;
	LEAP_CONNECTION ConnectionHandle;
	LEAP_TRACKING_EVENT *LastFrame = NULL;
	LEAP_IMAGE_FRAME_DESCRIPTION *ImageDescription = NULL;
	void* ImageBuffer = NULL;
	LEAP_DEVICE_INFO *LastDevice = NULL;
	
	FLeapWrapper();
	~FLeapWrapper();

	//Function Calls for plugin. Mainly uses Open/Close Connection.

	/** Set the LeapWrapperCallbackInterface delegate. Note that only one can be set at any time (static) */
	void SetCallbackDelegate(const LeapWrapperCallbackInterface* InCallbackDelegate);

	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	LEAP_CONNECTION* OpenConnection(const LeapWrapperCallbackInterface* InCallbackDelegate);

	/** Close the connection, it will nullify the callback delegate */
	void CloseConnection();

	void SetPolicy(int64 Flags, int64 ClearFlags);
	void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet);

	//Polling functions

	/** Get latest frame - critical section locked */
	LEAP_TRACKING_EVENT* GetFrame();

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp);

	LEAP_DEVICE_INFO* GetDeviceProperties(); //Used in polling example
	const char* ResultString(eLeapRS Result);

	void EnableImageStream(bool bEnable);

private:
	void CloseConnectionHandle(LEAP_CONNECTION* ConnectionHandle);
	void Millisleep(int Milliseconds);

	//Threading variables
	FCriticalSection DataLock;
	TFuture<void> ProducerLambdaFuture;
	static LeapWrapperCallbackInterface* CallbackDelegate;

	LEAP_TRACKING_EVENT* InterpolatedFrame;
	uint64 InterpolatedFrameSize;

	//TaskGraph event references are only stored to help with threading debug for now.
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

	//void setImage();
	void SetFrame(const LEAP_TRACKING_EVENT *Frame);
	void SetDevice(const LEAP_DEVICE_INFO *DeviceProps);
	void CleanupLastDevice();

	void ServiceMessageLoop(void * unused = nullptr);

	//Received LeapC callbacks converted into game thread events
	void HandleConnectionEvent(const LEAP_CONNECTION_EVENT *ConnectionEvent);
	void HandleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT *ConnectionLostEvent);
	void HandleDeviceEvent(const LEAP_DEVICE_EVENT *DeviceEvent);
	void HandleDeviceLostEvent(const LEAP_DEVICE_EVENT *DeviceEvent);
	void HandleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT *DeviceFailureEvent);
	void HandleTrackingEvent(const LEAP_TRACKING_EVENT *TrackingEvent);
	void HandleImageEvent(const LEAP_IMAGE_EVENT *ImageEvent);
	void HandleLogEvent(const LEAP_LOG_EVENT *LogEvent);
	void HandlePolicyEvent(const LEAP_POLICY_EVENT *PolicyEvent);
	void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT *ConfigChangeEvent);
	void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT *ConfigResponseEvent);
};
