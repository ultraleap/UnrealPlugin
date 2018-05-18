#pragma once

/** Interface for the passed callback delegate receiving game thread LeapC callbacks */
class LeapWrapperCallbackInterface
{
public:
	virtual void OnConnect() {};
	virtual void OnConnectionLost() {};
	virtual void OnDeviceFound(const LEAP_DEVICE_INFO *device) {};
	virtual void OnDeviceLost(const char* serial) {};
	virtual void OnDeviceFailure(
		const eLeapDeviceStatus failure_code,
		const LEAP_DEVICE failed_device) {};
	virtual void OnPolicy(const uint32_t current_policies) {};
	virtual void OnFrame(const LEAP_TRACKING_EVENT *tracking_event) {};
	virtual void OnImage(const LEAP_IMAGE_EVENT *image_event) {};
	virtual void OnLog(
		const eLeapLogSeverity severity,
		const int64_t timestamp,
		const char* message) {};
	virtual void OnConfigChange(const uint32_t requestID, const bool success) {};
	virtual void OnConfigResponse(const uint32_t requestID, LEAP_VARIANT value) {};
};

/** Wraps LeapC API into a threaded and event driven delegate callback format */
class FLeapWrapper
{
public:
	//LeapC Vars
	FThreadSafeBool bIsRunning;
	FThreadSafeBool bHasFinished;
	bool bIsConnected;
	LEAP_CONNECTION connectionHandle;
	LEAP_TRACKING_EVENT *lastFrame = NULL;
	LEAP_IMAGE_FRAME_DESCRIPTION *imageDescription = NULL;
	void* imageBuffer = NULL;
	LEAP_DEVICE_INFO *lastDevice = NULL;
	
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
	const char* ResultString(eLeapRS r);

	void EnableImageStream(bool bEnable);

private:
	void CloseConnectionHandle(LEAP_CONNECTION* connectionHandle);
	void millisleep(int milliseconds);

	//Threading variables
	FCriticalSection dataLock;
	class FLeapLambdaRunnable* ProducerLambdaThread;
	static LeapWrapperCallbackInterface* CallbackDelegate;

	LEAP_TRACKING_EVENT* interpolatedFrame;
	uint64 interpolatedFrameSize;

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
	void setFrame(const LEAP_TRACKING_EVENT *frame);
	void setDevice(const LEAP_DEVICE_INFO *deviceProps);
	void cleanupLastDevice();

	void serviceMessageLoop(void * unused = nullptr);

	//Received LeapC callbacks converted into game thread events
	void handleConnectionEvent(const LEAP_CONNECTION_EVENT *connection_event);
	void handleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT *connection_lost_event);
	void handleDeviceEvent(const LEAP_DEVICE_EVENT *device_event);
	void handleDeviceLostEvent(const LEAP_DEVICE_EVENT *device_event);
	void handleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT *device_failure_event);
	void handleTrackingEvent(const LEAP_TRACKING_EVENT *tracking_event);
	void handleImageEvent(const LEAP_IMAGE_EVENT *image_event);
	void handleLogEvent(const LEAP_LOG_EVENT *log_event);
	void handlePolicyEvent(const LEAP_POLICY_EVENT *policy_event);
	void handleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT *config_change_event);
	void handleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT *config_response_event);
};
