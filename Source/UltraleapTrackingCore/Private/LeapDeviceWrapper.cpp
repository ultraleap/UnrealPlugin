/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapDeviceWrapper.h"
#include "LeapAsync.h"
#include "LeapUtility.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

#pragma region Leap Device Wrapper

// created when a device is found
FLeapDeviceWrapper::FLeapDeviceWrapper(const uint32_t DeviceIDIn, const LEAP_DEVICE_INFO& DeviceInfoIn,
	const LEAP_DEVICE DeviceHandleIn, const LEAP_CONNECTION ConnectionHandleIn, IHandTrackingWrapper* ConnectorIn)
	:	
	DeviceID(DeviceIDIn)
	, DeviceHandle(DeviceHandleIn)
	, ConnectionHandle(ConnectionHandleIn)
	, DataLock(new FCriticalSection())
	, InterpolatedFrame(nullptr), InterpolatedFrameSize(0)
	, bIsRunning(false)
	, Connector(ConnectorIn)
{
	SetDevice(&DeviceInfoIn);

	Device = MakeShared<FUltraleapDevice>((IHandTrackingWrapper*) this, (ITrackingDeviceWrapper*) this);

}

FLeapDeviceWrapper::~FLeapDeviceWrapper()
{
	delete DataLock;
	DataLock = nullptr;
	
	bIsRunning = false;
	CallbackDelegate = nullptr;
	LatestFrame = nullptr;
	
	if (bIsConnected)
	{
		CloseConnection();
	}
	if (ImageDescription != NULL)
	{
		if (ImageDescription->pBuffer != NULL)
		{
			free(ImageDescription->pBuffer);
		}
		delete ImageDescription;
	}
}

void FLeapDeviceWrapper::SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate)
{
	// Should now be able to add connection directly to Lower level leapwrapper
	// which will callback the connection by device handle/ID
	CallbackDelegate = InCallbackDelegate;
	if (Connector)
	{
		Connector->SetCallbackDelegate(DeviceID, InCallbackDelegate);
	}
}

LEAP_CONNECTION* FLeapDeviceWrapper::OpenConnection(
	LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode)
{
	CallbackDelegate = InCallbackDelegate;
	bIsConnected = true;
	return nullptr;
}
void FLeapDeviceWrapper::CloseConnection()
{
	if (!bIsConnected)
	{
		// Not connected, already done
		// UE_LOG(UltraleapTrackingLog, Log, TEXT("Attempt at closing an already closed connection."));
		return;
	}
	bIsConnected = false;
	bIsRunning = false;
	CleanupLastDevice();
	
	// Wait for thread to exit - Blocking call, but it should be very quick.
	FTimespan ExitWaitTimeSpan = FTimespan::FromSeconds(3);

	ProducerLambdaFuture.WaitFor(ExitWaitTimeSpan);
	ProducerLambdaFuture.Reset();

	// Nullify the callback delegate. Any outstanding task graphs will not run if the delegate is nullified.
	CallbackDelegate = nullptr;

	UE_LOG(UltraleapTrackingLog, Log, TEXT("Connection successfully closed."));
}

void FLeapDeviceWrapper::SetTrackingMode(eLeapTrackingMode TrackingMode)
{
	if (!Connector)
	{
		return; 
	}
	Connector->SetTrackingModeEx(TrackingMode, DeviceID);
}
// Set policy is superceded by SetTrackingMode but still needed for images
void FLeapDeviceWrapper::SetPolicy(int64 Flags, int64 ClearFlags)
{
	if (!Connector)
	{
		return;
	}
	Connector->SetPolicyEx(Flags,ClearFlags, DeviceID);
}


void FLeapDeviceWrapper::SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet)
{
	if (ShouldSet)
	{
		SetPolicy(Flag, 0);
	}
	else
	{
		SetPolicy(0, Flag);
	}
}



LEAP_TRACKING_EVENT* FLeapDeviceWrapper::GetFrame()
{
	LEAP_TRACKING_EVENT* currentFrame;
	DataLock->Lock();
	currentFrame = LatestFrame;
	DataLock->Unlock();
	return currentFrame;
}

LEAP_TRACKING_EVENT* FLeapDeviceWrapper::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	uint64_t FrameSize = 0;
	eLeapRS Result = LeapGetFrameSizeEx(ConnectionHandle, DeviceHandle, TimeStamp, &FrameSize);
	
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapGetFrameSizeEx failed in  FLeapDeviceWrapper::GetInterpolatedFrameAtTime"));
		// if the device goes bad (currently due to system sleep wake and replug)
		// clean it up
		if (Connector)
		{
			Connector->CleanupBadDevice(this);
			Connector = nullptr;
			return nullptr;
		}
	}
	// Check validity of frame size
	if (FrameSize > 0)
	{
		// Different frame?
		if (FrameSize != InterpolatedFrameSize)
		{
			// If we already have an allocated frame, free it
			if (InterpolatedFrame)
			{
				free(InterpolatedFrame);
			}
			InterpolatedFrame = (LEAP_TRACKING_EVENT*) malloc(FrameSize);
		}
		InterpolatedFrameSize = FrameSize;

		// Grab the new frame
		Result = LeapInterpolateFrameEx(ConnectionHandle, DeviceHandle, TimeStamp, InterpolatedFrame, InterpolatedFrameSize);

		if (Result != eLeapRS_Success)
		{
			UE_LOG(UltraleapTrackingLog, Log,
				TEXT("LeapInterpolateFrameEx failed in  FLeapDeviceWrapper::GetInterpolatedFrameAtTime"));
		}
	}
	return InterpolatedFrame;
}

LEAP_DEVICE_INFO* FLeapDeviceWrapper::GetDeviceProperties()
{
	LEAP_DEVICE_INFO* currentDevice;
	DataLock->Lock();
	currentDevice = CurrentDeviceInfo;
	DataLock->Unlock();
	return currentDevice;
}

void FLeapDeviceWrapper::EnableImageStream(bool bEnable)
{
	if (ImageDescription == NULL)
	{
		ImageDescription = new LEAP_IMAGE_FRAME_DESCRIPTION;
		ImageDescription->pBuffer = NULL;
	}

	int OldLength = ImageDescription->buffer_len;

	// if the size is different realloc the buffer
	if (ImageDescription->buffer_len != OldLength)
	{
		if (ImageDescription->pBuffer != NULL)
		{
			free(ImageDescription->pBuffer);
		}
		ImageDescription->pBuffer = (void*) malloc(ImageDescription->buffer_len);
	}
}

void FLeapDeviceWrapper::Millisleep(int milliseconds)
{
	FPlatformProcess::Sleep(((float) milliseconds) / 1000.f);
}

void FLeapDeviceWrapper::SetDevice(const LEAP_DEVICE_INFO* DeviceProps)
{
	DataLock->Lock();
	if (CurrentDeviceInfo)
	{
		free(CurrentDeviceInfo->serial);
	}
	else
	{
		CurrentDeviceInfo = (LEAP_DEVICE_INFO*) malloc(sizeof(*DeviceProps));
	}
	*CurrentDeviceInfo = *DeviceProps;
	CurrentDeviceInfo->serial = (char*) malloc(DeviceProps->serial_length);
	memcpy(CurrentDeviceInfo->serial, DeviceProps->serial, DeviceProps->serial_length);
	DataLock->Unlock();
}

void FLeapDeviceWrapper::CleanupLastDevice()
{
	if (CurrentDeviceInfo)
	{
		free(CurrentDeviceInfo->serial);
	}
	CurrentDeviceInfo = nullptr;
	
	DeviceID = 0;
}

void FLeapDeviceWrapper::SetFrame(const LEAP_TRACKING_EVENT* Frame)
{
	DataLock->Lock();

	if (!LatestFrame)
	{
		LatestFrame = (LEAP_TRACKING_EVENT*) malloc(sizeof(*Frame));
	}

	*LatestFrame = *Frame;

	DataLock->Unlock();
}




/** Called by ServiceMessageLoop() when a tracking event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent)
{
	SetFrame(TrackingEvent);	// support polling tracking data from different thread
}

void FLeapDeviceWrapper::HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent)
{
	// Callback with data
	if (CallbackDelegate)
	{
		// Do image handling on background thread for performance
		CallbackDelegate->OnImage(ImageEvent);
	}
}

/** Called by ServiceMessageLoop() when a log event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandleLogEvent(const LEAP_LOG_EVENT* LogEvent)
{
	if (CallbackDelegate)
	{
		TaskRefLog = FLeapAsync::RunShortLambdaOnGameThread([LogEvent, this] {
			if (CallbackDelegate)
			{
				CallbackDelegate->OnLog(LogEvent->severity, LogEvent->timestamp, LogEvent->message);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a policy event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent)
{
	if (CallbackDelegate)
	{
		// this is always coming back as 0, this means either the Leap service refused to set any flags?
		// or there's a bug in the policy notification system with Leap Motion V4.
		const uint32_t CurrentPolicy = PolicyEvent->current_policy;
		TaskRefPolicy = FLeapAsync::RunShortLambdaOnGameThread([CurrentPolicy, this] {
			if (CallbackDelegate)
			{
				CallbackDelegate->OnPolicy(CurrentPolicy);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a policy event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingModeEvent)
{
	if (CallbackDelegate)
	{
		// this is always coming back as 0, this means either the Leap service refused to set any flags?
		// or there's a bug in the policy notification system with Leap Motion V4.
		const uint32_t CurrentMode = TrackingModeEvent->current_tracking_mode;
		TaskRefPolicy = FLeapAsync::RunShortLambdaOnGameThread([CurrentMode, this] {
			if (CallbackDelegate)
			{
				CallbackDelegate->OnTrackingMode((eLeapTrackingMode) CurrentMode);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a config change event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent)
{
	if (CallbackDelegate)
	{
		TaskRefConfigChange = FLeapAsync::RunShortLambdaOnGameThread([ConfigChangeEvent, this] {
			if (CallbackDelegate)
			{
				CallbackDelegate->OnConfigChange(ConfigChangeEvent->requestID, ConfigChangeEvent->status);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a config response event is returned by LeapPollConnection(). */
void FLeapDeviceWrapper::HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent)
{
	if (CallbackDelegate)
	{
		TaskRefConfigResponse = FLeapAsync::RunShortLambdaOnGameThread([ConfigResponseEvent, this] {
			if (CallbackDelegate)
			{
				CallbackDelegate->OnConfigResponse(ConfigResponseEvent->requestID, ConfigResponseEvent->value);
			}
		});
	}
}



#pragma endregion Leap Device Wrapper