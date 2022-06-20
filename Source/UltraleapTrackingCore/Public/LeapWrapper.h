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
#include "IUltraleapTrackingPlugin.h"

/** Interface for the passed callback delegate receiving game thread LeapC callbacks */
class LeapWrapperCallbackInterface
{
public:
	virtual void OnConnect(){};
	virtual void OnConnectionLost(){};
	virtual void OnDeviceFound(const LEAP_DEVICE_INFO* Device){};
	virtual void OnDeviceLost(const char* Serial){};
	virtual void OnDeviceFailure(const eLeapDeviceStatus FailureCode, const LEAP_DEVICE FailedDevice){};
	virtual void OnPolicy(const uint32_t CurrentPolicies){};
	virtual void OnTrackingMode(const eLeapTrackingMode current_tracking_mode){};
	virtual void OnFrame(const LEAP_TRACKING_EVENT* TrackingEvent){};
	virtual void OnImage(const LEAP_IMAGE_EVENT* ImageEvent){};
	virtual void OnLog(const eLeapLogSeverity Severity, const int64_t Timestamp, const char* Message){};
	virtual void OnConfigChange(const uint32_t RequestID, const bool Success){};
	virtual void OnConfigResponse(const uint32_t RequestID, LEAP_VARIANT Value){};
};
class IHandTrackingDevice
{
public:
	virtual void AddEventDelegate(const ULeapComponent* EventDelegate)  = 0;
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate)  = 0;
};
class IHandTrackingWrapper
{
public:
	virtual ~IHandTrackingWrapper()
	{
	}
	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode = false) = 0;

	/** Close the connection, it will nullify the callback delegate */
	virtual void CloseConnection() = 0;

	virtual void SetPolicy(int64 Flags, int64 ClearFlags) = 0;
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) = 0;
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) = 0;
	// Polling functions

	/** Get latest frame - critical section locked */
	virtual LEAP_TRACKING_EVENT* GetFrame() = 0;

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) = 0;

	virtual LEAP_DEVICE_INFO* GetDeviceProperties() = 0;
	
	virtual const char* ResultString(eLeapRS Result) = 0;

	virtual void EnableImageStream(bool bEnable) = 0;

	virtual bool IsConnected() = 0;

	virtual void SetWorld(UWorld* World) = 0;

	virtual int64_t GetNow() = 0;

	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) = 0;

	virtual uint32_t GetDeviceID() = 0;

	virtual void SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate) = 0;
	virtual void SetCallbackDelegate(const uint32_t DeviceID, LeapWrapperCallbackInterface* InCallbackDelegate) = 0;

	virtual FString GetDeviceSerial() = 0;

	virtual IHandTrackingDevice* GetDevice() = 0;
};

class FLeapWrapperBase : public IHandTrackingWrapper
{
public:
	LEAP_DEVICE_INFO* CurrentDeviceInfo = NULL;
	bool bIsConnected = false;

	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode)
	{
		CallbackDelegate = InCallbackDelegate;
		return nullptr;
	}

	/** Close the connection, it will nullify the callback delegate */
	virtual void CloseConnection() override
	{
	}

	virtual void SetPolicy(int64 Flags, int64 ClearFlags) override
	{
	}
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) override
	{
	}
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) override
	{
	}
	// Polling functions

	/** Get latest frame - critical section locked */
	virtual LEAP_TRACKING_EVENT* GetFrame() override
	{
		return nullptr;
	}

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) override
	{
		return nullptr;
	}

	virtual LEAP_DEVICE_INFO* GetDeviceProperties() override
	{
		return nullptr;
	}
	virtual const char* ResultString(eLeapRS Result) override
	{
		return nullptr;
	}

	virtual void EnableImageStream(bool bEnable) override
	{
	}
	virtual bool IsConnected() override
	{
		return bIsConnected;
	}

	virtual void SetWorld(UWorld* World) override
	{
		CurrentWorld = World;
	}

	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) override
	{
	}
	virtual uint32_t GetDeviceID() override
	{
		return 0;
	}
	virtual void SetCallbackDelegate(const uint32_t DeviceID, LeapWrapperCallbackInterface* InCallbackDelegate) override
	{
		return;
	}
	virtual void SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate) override
	{
		return;
	}
	virtual FString GetDeviceSerial() override
	{
		return TEXT("Unknown");
	}
	virtual IHandTrackingDevice* GetDevice() override
	{
		return nullptr;
	}
protected:
	LeapWrapperCallbackInterface* CallbackDelegate = nullptr;
	UWorld* CurrentWorld = nullptr;
};
/** Wraps LeapC API into a threaded and event driven delegate callback format */
class FLeapWrapper : public IHandTrackingWrapper, public ILeapConnector
{
public:
	// LeapC Vars
	FThreadSafeBool bIsRunning;
	FThreadSafeBool bHasFinished;

	LEAP_CONNECTION ConnectionHandle;
	LEAP_IMAGE_FRAME_DESCRIPTION* ImageDescription = NULL;
	void* ImageBuffer = NULL;

	FLeapWrapper();
	virtual ~FLeapWrapper();

	// Function Calls for plugin. Mainly uses Open/Close Connection.
	// IHandTrackingWrapper
	/** Set the LeapWrapperCallbackInterface delegate. Note that only one can be set at any time (static) */
	void SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate) override;
	void SetCallbackDelegate(const uint32_t DeviceID, LeapWrapperCallbackInterface* InCallbackDelegate) override;
	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode) override;

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

	virtual LEAP_DEVICE_INFO* GetDeviceProperties() override
	{
		return nullptr;
	}
	virtual const char* ResultString(eLeapRS Result) override;

	virtual void EnableImageStream(bool bEnable) override;

	virtual bool IsConnected() override
	{
		return bIsConnected;
	}

	virtual void SetWorld(UWorld* World)
	{
	}
	virtual uint32_t GetDeviceID() override
	{
		return 0;
	}
	
	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW){};
	
	virtual int64_t GetNow() override
	{
		return LeapGetNow();
	}
	virtual FString GetDeviceSerial() override
	{
		return TEXT("LeapWrapper/Connector");
	}
	virtual IHandTrackingDevice* GetDevice() override
	{
		return nullptr;
	}
	// ILeapConnector
	virtual void GetDeviceSerials(TArray<FString>& DeviceSerials) override;
	virtual IHandTrackingWrapper* GetDevice(const TArray<FString>& DeviceSerial) override;

private:
	void CloseConnectionHandle(LEAP_CONNECTION* ConnectionHandle);
	void Millisleep(int Milliseconds);

	LeapWrapperCallbackInterface* ConnectorCallbackDelegate = nullptr;
	TMap<uint32_t, LeapWrapperCallbackInterface*> MapDeviceToCallback;
	TMap<uint32_t, LEAP_DEVICE> MapDeviceIDToDevice;

	LeapWrapperCallbackInterface* GetCallbackDelegateFromDeviceID(const uint32_t DeviceID);
	
	// Frame and handle data
	TArray <IHandTrackingWrapper* > Devices;
	LEAP_TRACKING_EVENT* LatestFrame = NULL;

	// Threading variables
	FCriticalSection* DataLock;
	TFuture<void> ProducerLambdaFuture;

	LEAP_TRACKING_EVENT* InterpolatedFrame;
	uint64 InterpolatedFrameSize;

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

	// void setImage();
	void SetFrame(const LEAP_TRACKING_EVENT* Frame);
	//void SetDevice(const LEAP_DEVICE_INFO* DeviceProps);
	

	void ServiceMessageLoop(void* unused = nullptr);

	// Received LeapC callbacks converted into game thread events
	void HandleConnectionEvent(const LEAP_CONNECTION_EVENT* ConnectionEvent);
	void HandleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT* ConnectionLostEvent);
	void HandleDeviceEvent(const LEAP_DEVICE_EVENT* DeviceEvent);
	void HandleDeviceLostEvent(const LEAP_DEVICE_EVENT* DeviceEvent);
	void HandleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT* DeviceFailureEvent, const uint32_t DeviceID);
	void HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent, const uint32_t DeviceID);
	void HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent, const uint32_t DeviceID);
	void HandleLogEvent(const LEAP_LOG_EVENT* LogEvent, const uint32_t DeviceID);
	void HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent, const uint32_t DeviceID);
	void HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingEvent, const uint32_t DeviceID);
	void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent, const uint32_t DeviceID);
	void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent, const uint32_t DeviceID);

	bool bIsConnected = false;

	void AddDevice(const uint32_t DeviceID, const LEAP_DEVICE_INFO& DeviceInfo);
	void RemoveDevice(const uint32_t DeviceID);
};
