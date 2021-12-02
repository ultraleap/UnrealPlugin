

#pragma once
#include "Async/Async.h"
#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"
#include "LeapC.h"

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
UENUM()
enum class ELeapQuatSwizzleAxisB : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z"),
	W UMETA(DisplayName = "W"),
	MinusX UMETA(DisplayName = "-X"),
	MinusY UMETA(DisplayName = "-Y"),
	MinusZ UMETA(DisplayName = "-Z"),
	MinusW UMETA(DisplayName = "-W")
};

class IHandTrackingWrapper
{
public:
	virtual ~IHandTrackingWrapper()
	{
	}
	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate) = 0;

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

	virtual LEAP_DEVICE_INFO* GetDeviceProperties() = 0;	// Used in polling example
	virtual const char* ResultString(eLeapRS Result) = 0;

	virtual void EnableImageStream(bool bEnable) = 0;

	virtual bool IsConnected() = 0;

	virtual void SetWorld(UWorld* World) = 0;

	virtual int64_t GetNow() = 0;

	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) = 0;
};

class FLeapWrapperBase : public IHandTrackingWrapper
{
public:
	LEAP_DEVICE_INFO* CurrentDeviceInfo = NULL;
	bool bIsConnected = false;

	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate)
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

protected:
	LeapWrapperCallbackInterface* CallbackDelegate = nullptr;
	UWorld* CurrentWorld = nullptr;
};
/** Wraps LeapC API into a threaded and event driven delegate callback format */
class FLeapWrapper : public FLeapWrapperBase
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

	/** Set the LeapWrapperCallbackInterface delegate. Note that only one can be set at any time (static) */
	void SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate);

	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate) override;

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
	virtual const char* ResultString(eLeapRS Result) override;

	virtual void EnableImageStream(bool bEnable) override;
	virtual int64_t GetNow() override
	{
		return LeapGetNow();
	}

private:
	void CloseConnectionHandle(LEAP_CONNECTION* ConnectionHandle);
	void Millisleep(int Milliseconds);

	// Frame and handle data
	LEAP_DEVICE DeviceHandle;
	LEAP_TRACKING_EVENT* LatestFrame = NULL;

	// Threading variables
	FCriticalSection DataLock;
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
	void SetDevice(const LEAP_DEVICE_INFO* DeviceProps);
	void CleanupLastDevice();

	void ServiceMessageLoop(void* unused = nullptr);

	// Received LeapC callbacks converted into game thread events
	void HandleConnectionEvent(const LEAP_CONNECTION_EVENT* ConnectionEvent);
	void HandleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT* ConnectionLostEvent);
	void HandleDeviceEvent(const LEAP_DEVICE_EVENT* DeviceEvent);
	void HandleDeviceLostEvent(const LEAP_DEVICE_EVENT* DeviceEvent);
	void HandleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT* DeviceFailureEvent);
	void HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent);
	void HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent);
	void HandleLogEvent(const LEAP_LOG_EVENT* LogEvent);
	void HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent);
	void HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingEvent);
	void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent);
	void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent);
};
