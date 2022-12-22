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


class FLeapWrapperBase : public IHandTrackingWrapper, public ITrackingDeviceWrapper
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
	virtual void SetPolicyEx(int64 Flags, int64 ClearFlags, const uint32_t DeviceID = 0) override
	{

	}
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) override
	{
	}
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) override
	{
	}
	virtual void SetTrackingModeEx(eLeapTrackingMode TrackingMode, const uint32_t DeviceID = 0) override
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
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTimeEx(int64 TimeStamp, const uint32_t DeviceID = 0) override
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
	virtual EDeviceType GetDeviceType() override
	{
		return DEVICE_TYPE_LEAP;
	}
	virtual IHandTrackingDevice* GetDevice() override
	{
		return nullptr;
	}

	// ITrackingDeviceWrapper
	virtual void HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent) override
	{
	}
	virtual void HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent) override
	{
	}
	virtual void HandleLogEvent(const LEAP_LOG_EVENT* LogEvent) override
	{
	}
	virtual void HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent) override
	{
	}
	virtual void HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingEvent) override
	{
	}
	virtual void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent) override
	{
	}
	virtual void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent) override
	{
	}
	virtual bool MatchDevices(const TArray<FString> DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass) override
	{
		return false;
	}
	virtual bool ContainsDevice(IHandTrackingWrapper* DeviceWrapper) override
	{
		return false;
	}
	virtual void CleanupBadDevice(IHandTrackingWrapper* DeviceWrapper) override
	{
	
	}
	virtual void PostLeapHandUpdate(FLeapFrameData& Frame) override
	{
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
	virtual void SetPolicyEx(int64 Flags, int64 ClearFlags, const uint32_t DeviceID = 0) override;
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) override;
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) override;
	virtual void SetTrackingModeEx(eLeapTrackingMode TrackingMode, const uint32_t DeviceID = 0) override;
	// Polling functions

	/** Get latest frame - critical section locked */
	virtual LEAP_TRACKING_EVENT* GetFrame() override;

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) override;
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTimeEx(int64 TimeStamp, const uint32_t DeviceID = 0) override;

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
	virtual EDeviceType GetDeviceType() override
	{
		return DEVICE_TYPE_LEAP;
	}
	virtual IHandTrackingDevice* GetDevice() override
	{
		return nullptr;
	}
	virtual bool MatchDevices(const TArray<FString> DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass) override
	{
		return false;
	}
	virtual bool ContainsDevice(IHandTrackingWrapper* DeviceWrapper) override
	{
		return false;
	}
	virtual void CleanupBadDevice(IHandTrackingWrapper* DeviceWrapper) override;
	virtual void PostLeapHandUpdate(FLeapFrameData& Frame) override
	{
	}
	// ILeapConnector
	virtual void GetDeviceSerials(TArray<FString>& DeviceSerials) override;
	virtual IHandTrackingWrapper* GetDevice(
		const TArray<FString>& DeviceSerial, const ELeapDeviceCombinerClass DeviceCombinerClass, const bool AllowOpenXRAsFallback) override;
	virtual void TickDevices(const float DeltaTime);
	virtual void TickSendControllerEventsOnDevices();
	virtual ELeapDeviceType GetDeviceTypeFromSerial(const FString& DeviceSerial) override;
	virtual void AddLeapConnectorCallback(ILeapConnectorCallbacks* Callback) override;
	virtual void RemoveLeapConnnectorCallback(ILeapConnectorCallbacks* Callback) override;
	virtual void PostEarlyInit() override;
	// End of ILeapConnector

private:
	void CloseConnectionHandle(LEAP_CONNECTION* ConnectionHandle);
	void Millisleep(int Milliseconds);

	LeapWrapperCallbackInterface* ConnectorCallbackDelegate = nullptr;
	TMap<uint32_t, LeapWrapperCallbackInterface*> MapDeviceToCallback;
	TMap<uint32_t, LEAP_DEVICE> MapDeviceIDToDevice;

	LeapWrapperCallbackInterface* GetCallbackDelegateFromDeviceID(const uint32_t DeviceID);
	
	// Frame and handle data
	// Actual connected devices
	TArray <IHandTrackingWrapper*> Devices;
	TArray<IHandTrackingWrapper*> DevicesToCleanup;
	// Aggregated/combined devices
	TArray<IHandTrackingWrapper*> CombinedDevices;

	TArray<ILeapConnectorCallbacks*> LeapConnectorCallbacks;

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
	bool UseOpenXR = false;

	void AddDevice(const uint32_t DeviceID, const LEAP_DEVICE_INFO& DeviceInfo, const LEAP_DEVICE DeviceHandle);
	void RemoveDevice(const uint32_t DeviceID);

	void AddOpenXRDevice(LeapWrapperCallbackInterface* InCallbackDelegate);

	IHandTrackingWrapper* GetSingularDeviceBySerial(const FString& DeviceSerial);
	LEAP_DEVICE GetDeviceHandleFromDeviceID(const uint32_t DeviceID);
	
	IHandTrackingWrapper* FindAggregator(const TArray<FString>& DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass);
	IHandTrackingWrapper* CreateAggregator(
		const TArray<FString>& DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass);
	
	void NotifyDeviceAdded(IHandTrackingWrapper* Device);
	void NotifyDeviceRemoved(IHandTrackingWrapper* Device);
	void CleanupCombinedDevicesReferencingDevice(IHandTrackingWrapper* Device);
	void RemoveDeviceDirect(const uint32_t DeviceID);

};
