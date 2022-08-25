/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "IInputDeviceModule.h"
#include "UltraleapTrackingData.h"

class ULeapComponent;



class IHandTrackingDevice
{
public:
	virtual void AddEventDelegate(const ULeapComponent* EventDelegate) = 0;
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate) = 0;

	virtual void Tick(const float DeltaTime) = 0;
	virtual void SendControllerEvents() = 0;

	virtual void GetLatestFrameData(FLeapFrameData& OutData, const bool ApplyDeviceOrigin  = false) = 0;
	virtual void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible) = 0;
	virtual void SetOptions(const FLeapOptions& InOptions) = 0;
	virtual FLeapOptions GetOptions() = 0;
	virtual FLeapStats GetStats() = 0;
	virtual ELeapDeviceType GetDeviceType() = 0;
	virtual FTransform& GetDeviceOrigin() = 0;
	virtual void SetDeviceOrigin(const FTransform& DeviceOriginIn) = 0;
	virtual void UpdateJointOcclusions(class AJointOcclusionActor* Actor) = 0;
	virtual bool GetJointOcclusionConfidences(const FString& DeviceSerial, TArray<float>& Left, TArray<float>& Right) = 0;
	virtual void GetDebugInfo(int32& NumCombinedLeft, int32& NumCombinedRight) = 0;
	virtual int32 GetBodyStateDeviceID() = 0;
};
class ITrackingDeviceWrapper
{
public:
	virtual void HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent) = 0;
	virtual void HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent) = 0;
	virtual void HandleLogEvent(const LEAP_LOG_EVENT* LogEvent) = 0;
	virtual void HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent) = 0;
	virtual void HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingEvent) = 0;
	virtual void HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent) = 0;
	virtual void HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent) = 0;
};

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
class IHandTrackingWrapper
{
public:
	enum EDeviceType
	{
		DEVICE_TYPE_LEAP,
		DEVICE_TYPE_OPENXR
	};

	virtual ~IHandTrackingWrapper()
	{
	}
	/** Open the connection and set our static LeapWrapperCallbackInterface delegate */
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode = false) = 0;

	/** Close the connection, it will nullify the callback delegate */
	virtual void CloseConnection() = 0;

	virtual void SetPolicy(int64 Flags, int64 ClearFlags) = 0;
	virtual void SetPolicyEx(int64 Flags, int64 ClearFlags, const uint32_t DeviceID = 0) = 0;
	virtual void SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet) = 0;
	// Supercedes SetPolicy for HMD/Desktop/Screentop modes
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) = 0;
	virtual void SetTrackingModeEx(eLeapTrackingMode TrackingMode, const uint32_t DeviceID = 0) = 0;
	// Polling functions

	/** Get latest frame - critical section locked */
	virtual LEAP_TRACKING_EVENT* GetFrame() = 0;

	/** Uses leap method to get an interpolated frame at a given leap timestamp in microseconds given by e.g. LeapGetNow()*/
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) = 0;
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTimeEx(int64 TimeStamp, const uint32_t DeviceID = 0) = 0;
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
	virtual EDeviceType GetDeviceType() = 0;
	virtual IHandTrackingDevice* GetDevice() = 0;

	// device combination, does this device aggregate the given Devices
	virtual bool MatchDevices(const TArray<FString> DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass) = 0;
	virtual bool ContainsDevice(IHandTrackingWrapper* DeviceWrapper) = 0;
	virtual void CleanupBadDevice(IHandTrackingWrapper* DeviceWrapper) = 0;
	// apply any post frame processing
	virtual void PostLeapHandUpdate(FLeapFrameData& Frame) = 0;
};
class ILeapConnectorCallbacks
{
public:
	virtual void OnDeviceAdded(IHandTrackingWrapper* DeviceWrapper) = 0;
	// call before device cleaned up
	virtual void OnDeviceRemoved(IHandTrackingWrapper* DeviceWrapper) = 0;
};
class ILeapConnector
{
public:
	// get available/active tracking devices
	virtual void GetDeviceSerials(TArray<FString>& DeviceSerials) = 0;
	// todo: add aggregator/combiner class
	// if in singular mode pass one tracking device serial
	virtual class IHandTrackingWrapper* GetDevice(const TArray<FString>& DeviceSerial,
		const ELeapDeviceCombinerClass DeviceCombinerClass, const bool AllowOpenXRAsFallback) = 0;

	virtual void TickDevices(const float DeltaTime) = 0;
	virtual void TickSendControllerEventsOnDevices() = 0;

	virtual ELeapDeviceType GetDeviceTypeFromSerial(const FString& DeviceSerial) = 0;

	virtual void AddLeapConnectorCallback(ILeapConnectorCallbacks* Callback) = 0;
	virtual void RemoveLeapConnnectorCallback(ILeapConnectorCallbacks* Callback) = 0;
	// called when the engine is ready for input
	virtual void PostEarlyInit() = 0;
};
/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class ULTRALEAPTRACKING_API IUltraleapTrackingPlugin : public IInputDeviceModule
{
public:
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IUltraleapTrackingPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<IUltraleapTrackingPlugin>("UltraleapTracking");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UltraleapTracking");
	}

	// These are typically called by a wrapped class such as LeapController (Actor Component)

	/** Attach an event delegate to the leap input device loop*/
	virtual void AddEventDelegate(const ULeapComponent* EventDelegate){};

	/** Remove an event delegate from the leap input device loop*/
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate){};

	virtual FLeapStats GetLeapStats(const FString& DeviceSerial)
	{
		return FLeapStats();
	};

	/** Set Leap Options such as time warp, interpolation and tracking modes */
	virtual void SetOptions(const FLeapOptions& InOptions, const TArray<FString>& DeviceSerials){};

	/** Get the currently set Leap Options */
	virtual FLeapOptions GetOptions(const FString& DeviceSerial)
	{
		return FLeapOptions();
	};

	/** Convenience function to determine hand visibility*/
	virtual void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible, const FString& DeviceSerial) = 0;

	/** Polling method for latest frame data*/
	virtual void GetLatestFrameData(FLeapFrameData& OutData, const FString& DeviceSerial) = 0;

	/** Set a Leap Policy, such as image streaming or optimization type*/
	virtual void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials) = 0;

	/** List the attached (plugged in) devices */
	virtual void GetAttachedDevices(TArray<FString>& Devices) = 0;

	/** Force shutdown leap, do not call unless you have a very specfic need*/
	virtual void ShutdownLeap() = 0;

	virtual void SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ,
		ELeapQuatSwizzleAxisB ToW, const TArray<FString>& DeviceSerials) = 0;

	virtual ILeapConnector* GetConnector() = 0;
};
