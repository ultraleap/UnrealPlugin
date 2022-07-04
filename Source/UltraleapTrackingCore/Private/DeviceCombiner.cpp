/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "DeviceCombiner.h"
#include "LeapAsync.h"
#include "LeapUtility.h"
#include "FUltraleapCombinedDevice.h"
#include "FUltraleapCombinedDeviceAngular.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

#pragma region Combiner

// created when a device is found
FDeviceCombiner::FDeviceCombiner(const LEAP_CONNECTION ConnectionHandleIn, IHandTrackingWrapper* ConnectorIn,
	const TArray<IHandTrackingWrapper*>& DevicesToCombineIn, const ELeapDeviceCombinerClass DeviceCombinerClass)
	:
	ConnectionHandle(ConnectionHandleIn)
	, DataLock(new FCriticalSection())
	, bIsRunning(false)
	, Connector(ConnectorIn)
	, DevicesToCombine(DevicesToCombineIn)
{
	//SetDevice(&DeviceInfoIn);
	CombinedDeviceSerial = "Combined - ";

	// list of last 4 chars of the serial number
	for (auto DeviceToCombine : DevicesToCombineIn)
	{
		CombinedDeviceSerial += DeviceToCombine->GetDeviceSerial().Right(4);
		CombinedDeviceSerial += " ";
	}
	// create a new combiner otherwise
	switch (DeviceCombinerClass)
	{
		case ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_CONFIDENCE:
		{
			Device = MakeShared<FUltraleapCombinedDeviceAngular>(
				(IHandTrackingWrapper*) this, (ITrackingDeviceWrapper*) this, DevicesToCombineIn);
			break;
		}
		case ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_ANGULAR:
		{
			Device = MakeShared<FUltraleapCombinedDeviceAngular>(
				(IHandTrackingWrapper*) this, (ITrackingDeviceWrapper*) this, DevicesToCombineIn);
			break;
		}
		default:
			Device = MakeShared<FUltraleapCombinedDeviceAngular>(
				(IHandTrackingWrapper*) this, (ITrackingDeviceWrapper*) this, DevicesToCombineIn);
	}
}

FDeviceCombiner::~FDeviceCombiner()
{
	delete DataLock;
	DataLock = nullptr;
	
	bIsRunning = false;
	CallbackDelegate = nullptr;
//	LatestFrame = nullptr;
	
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

void FDeviceCombiner::SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate)
{
	// Should now be able to add connection directly to Lower level leapwrapper
	// which will callback the connection by device handle/ID
	CallbackDelegate = InCallbackDelegate;
	if (Connector)
	{
		Connector->SetCallbackDelegate(DeviceID, InCallbackDelegate);
	}
}

LEAP_CONNECTION* FDeviceCombiner::OpenConnection(
	LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode)
{
	CallbackDelegate = InCallbackDelegate;
	bIsConnected = true;
	return nullptr;
}
void FDeviceCombiner::CloseConnection()
{
	if (!bIsConnected)
	{
		// Not connected, already done
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Attempt at closing an already closed connection."));
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

void FDeviceCombiner::SetTrackingMode(eLeapTrackingMode TrackingMode)
{
	if (!Connector)
	{
		return; 
	}
	Connector->SetTrackingModeEx(TrackingMode, DeviceID);
}
// Set policy is superceded by SetTrackingMode but still needed for images
void FDeviceCombiner::SetPolicy(int64 Flags, int64 ClearFlags)
{
	if (!Connector)
	{
		return;
	}
	Connector->SetPolicyEx(Flags,ClearFlags, DeviceID);
}


void FDeviceCombiner::SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet)
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



LEAP_TRACKING_EVENT* FDeviceCombiner::GetFrame()
{
	/* LEAP_TRACKING_EVENT* currentFrame;
	DataLock->Lock();
	currentFrame = LatestFrame;
	DataLock->Unlock();
	return currentFrame;*/
	return nullptr;
}

LEAP_TRACKING_EVENT* FDeviceCombiner::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	/* uint64_t FrameSize = 0;
	eLeapRS Result = LeapGetFrameSizeEx(ConnectionHandle, DeviceHandle, TimeStamp, &FrameSize);
	
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapGetFrameSizeEx failed in  FDeviceCombiner::GetInterpolatedFrameAtTime"));
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
				TEXT("LeapInterpolateFrameEx failed in  FDeviceCombiner::GetInterpolatedFrameAtTime"));
		}
	}
	return InterpolatedFrame;*/
	return nullptr;
}

LEAP_DEVICE_INFO* FDeviceCombiner::GetDeviceProperties()
{
	LEAP_DEVICE_INFO* currentDevice;
	DataLock->Lock();
	currentDevice = CurrentDeviceInfo;
	DataLock->Unlock();
	return currentDevice;
}

void FDeviceCombiner::EnableImageStream(bool bEnable)
{
	// TODO: test the image/buffer stream code

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

void FDeviceCombiner::Millisleep(int milliseconds)
{
	FPlatformProcess::Sleep(((float) milliseconds) / 1000.f);
}


void FDeviceCombiner::CleanupLastDevice()
{
}




/** Called by ServiceMessageLoop() when a tracking event is returned by LeapPollConnection(). */
void FDeviceCombiner::HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent)
{
	//SetFrame(TrackingEvent);	// support polling tracking data from different thread
}

void FDeviceCombiner::HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent)
{
	// Callback with data
	if (CallbackDelegate)
	{
		// Do image handling on background thread for performance
		CallbackDelegate->OnImage(ImageEvent);
	}
}

/** Called by ServiceMessageLoop() when a log event is returned by LeapPollConnection(). */
void FDeviceCombiner::HandleLogEvent(const LEAP_LOG_EVENT* LogEvent)
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
void FDeviceCombiner::HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent)
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
void FDeviceCombiner::HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingModeEvent)
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
void FDeviceCombiner::HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent)
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
void FDeviceCombiner::HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent)
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
bool FDeviceCombiner::MatchDevices(const TArray<FString> DeviceSerials)
{
	if (DevicesToCombine.Num() != DeviceSerials.Num())
	{
		return false;
	}
	for (auto DeviceToCombine : DevicesToCombine)
	{
		if (!DeviceSerials.Contains(DeviceToCombine->GetDeviceSerial()))
		{
			return false;
		}
	}
	return true;
}


#pragma endregion Leap Device Wrapper