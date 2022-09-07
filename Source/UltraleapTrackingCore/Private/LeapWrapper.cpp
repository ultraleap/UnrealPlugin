/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapWrapper.h"
#include "LeapDeviceWrapper.h"
#include "LeapAsync.h"
#include "LeapUtility.h"
#include "Multileap/DeviceCombiner.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

#pragma region LeapC Wrapper

FLeapWrapper::FLeapWrapper()
	: bIsRunning(false)
	, DataLock(new FCriticalSection())
	, InterpolatedFrame(nullptr)
	, InterpolatedFrameSize(0)
{
	UseOpenXR = true;
}

FLeapWrapper::~FLeapWrapper()
{
	for (auto CombinedDevice : CombinedDevices)
	{
		delete CombinedDevice;
	}
	CombinedDevices.Empty();
	for (auto Device : Devices)
	{
		delete Device;
	}
	Devices.Empty();
	bIsRunning = false;
	// map device to callback delegate
	MapDeviceToCallback.Empty();

	ConnectionHandle = nullptr;

	if (bIsConnected)
	{
		CloseConnection();
	}
}
// to be deprecated
void FLeapWrapper::SetCallbackDelegate(LeapWrapperCallbackInterface* InCallbackDelegate)
{
	// fallback behaviour for non multidevice
	MapDeviceToCallback.Add(0, InCallbackDelegate);
	ConnectorCallbackDelegate = InCallbackDelegate;
}
// per device event handling
void FLeapWrapper::SetCallbackDelegate(const uint32_t DeviceID, LeapWrapperCallbackInterface* InCallbackDelegate)
{
	MapDeviceToCallback.Add(DeviceID, InCallbackDelegate);
}
LEAP_CONNECTION* FLeapWrapper::OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode)
{
	ConnectorCallbackDelegate = InCallbackDelegate;
	// Don't use config for now
	LEAP_CONNECTION_CONFIG Config = {0};
	
	Config.server_namespace = "Leap Service";
	Config.size = sizeof(Config);
	Config.flags = 0;
	
	if (UseMultiDeviceMode)
	{
		Config.flags = _eLeapConnectionConfig::eLeapConnectionConfig_MultiDeviceAware;
	}

	eLeapRS result = LeapCreateConnection(&Config, &ConnectionHandle);
	if (result == eLeapRS_Success)
	{
		result = LeapOpenConnection(ConnectionHandle);
		if (result == eLeapRS_Success)
		{
			bIsRunning = true;

			LEAP_CONNECTION* Handle = &ConnectionHandle;
			ProducerLambdaFuture = FLeapAsync::RunLambdaOnBackGroundThread([&, Handle] {
				UE_LOG(UltraleapTrackingLog, Log, TEXT("ServiceMessageLoop started."));
				ServiceMessageLoop();
				UE_LOG(UltraleapTrackingLog, Log, TEXT("ServiceMessageLoop stopped."));

				CloseConnectionHandle(Handle);
			});
		}
	}
	
	return &ConnectionHandle;
}
LeapWrapperCallbackInterface* FLeapWrapper::GetCallbackDelegateFromDeviceID(const uint32_t DeviceID)
{
	if(MapDeviceToCallback.Contains(DeviceID))
	{
		return *MapDeviceToCallback.Find(DeviceID);
	}
	return nullptr;
}

void FLeapWrapper::CloseConnection()
{
	if (!bIsConnected)
	{
		// Not connected, already done
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Attempt at closing an already closed connection."));
		return;
	}
	bIsConnected = false;
	bIsRunning = false;
	

	// Wait for thread to exit - Blocking call, but it should be very quick.
	FTimespan ExitWaitTimeSpan = FTimespan::FromSeconds(3);

	ProducerLambdaFuture.WaitFor(ExitWaitTimeSpan);
	ProducerLambdaFuture.Reset();

	// Nullify the callback delegate. Any outstanding task graphs will not run if the delegate is nullified.
	MapDeviceToCallback.Empty();

	UE_LOG(UltraleapTrackingLog, Log, TEXT("Connection successfully closed."));
}
void FLeapWrapper::SetTrackingMode(eLeapTrackingMode TrackingMode)
{
	eLeapRS Result = LeapSetTrackingMode(ConnectionHandle, TrackingMode);
	
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("SetTrackingMode failed in  FLeapWrapper::SetTrackingMode."));
	}
}
LEAP_DEVICE FLeapWrapper::GetDeviceHandleFromDeviceID(const uint32_t DeviceID)
{
	LEAP_DEVICE DeviceHandle = nullptr;
	if (MapDeviceIDToDevice.Contains(DeviceID))
	{
		DeviceHandle = MapDeviceIDToDevice[DeviceID];
	}
	return DeviceHandle;
}
void FLeapWrapper::SetTrackingModeEx(eLeapTrackingMode TrackingMode, const uint32_t DeviceID /* = 0*/)
{
	if (!DeviceID)
	{
		SetTrackingMode(TrackingMode);
	}
	LEAP_DEVICE DeviceHandle = GetDeviceHandleFromDeviceID(DeviceID);

	if (!DeviceHandle)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("SetTrackingModeEx failed cannot find device handle"));
	}
	eLeapRS Result = LeapSetTrackingModeEx(ConnectionHandle, DeviceHandle, TrackingMode);
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("SetTrackingModeEx failed in  FLeapWrapper::SetTrackingModeEx."));
	}
}
void FLeapWrapper::SetPolicy(int64 Flags, int64 ClearFlags)
{
	eLeapRS Result = LeapSetPolicyFlags(ConnectionHandle, Flags, ClearFlags);
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapSetPolicyFlags failed in  FLeapWrapper::SetPolicy."));
	}
}
void FLeapWrapper::SetPolicyEx(int64 Flags, int64 ClearFlags, const uint32_t DeviceID)
{
	if (!DeviceID)
	{
		SetPolicy(Flags, ClearFlags);
	}
	LEAP_DEVICE DeviceHandle = GetDeviceHandleFromDeviceID(DeviceID);
	if (!DeviceHandle)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("SetPolicyEx failed cannot find device handle"));
	}
	eLeapRS Result = LeapSetPolicyFlagsEx(ConnectionHandle, DeviceHandle, Flags, ClearFlags);
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapSetPolicyFlags failed in  FLeapWrapper::SetPolicyEx."));
	}
}
void FLeapWrapper::SetPolicyFlagFromBoolean(eLeapPolicyFlag Flag, bool ShouldSet)
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

/** Close the connection and let message thread function end. */
void FLeapWrapper::CloseConnectionHandle(LEAP_CONNECTION* InConnectionHandle)
{
	bIsRunning = false;
	bIsConnected = false;
	LeapDestroyConnection(*InConnectionHandle);
}

LEAP_TRACKING_EVENT* FLeapWrapper::GetFrame()
{
	LEAP_TRACKING_EVENT* currentFrame;
	DataLock->Lock();
	currentFrame = LatestFrame;
	DataLock->Unlock();
	return currentFrame;
}

LEAP_TRACKING_EVENT* FLeapWrapper::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	uint64_t FrameSize = 0;

	eLeapRS Result = LeapGetFrameSize(ConnectionHandle, TimeStamp, &FrameSize);

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
		LeapInterpolateFrame(ConnectionHandle, TimeStamp, InterpolatedFrame, InterpolatedFrameSize);
	}

	return InterpolatedFrame;
}
LEAP_TRACKING_EVENT* FLeapWrapper::GetInterpolatedFrameAtTimeEx(int64 TimeStamp, const uint32_t DeviceID)
{
	if (!DeviceID)
	{
		return GetInterpolatedFrameAtTime(TimeStamp);
	}
	uint64_t FrameSize = 0;
	LEAP_DEVICE DeviceHandle = nullptr;

	DeviceHandle = GetDeviceHandleFromDeviceID(DeviceID);
	eLeapRS Result = LeapGetFrameSizeEx(ConnectionHandle, DeviceHandle, TimeStamp, &FrameSize );

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
		LeapInterpolateFrameEx(ConnectionHandle,DeviceHandle,TimeStamp, InterpolatedFrame, InterpolatedFrameSize);
	}

	return InterpolatedFrame;
}

/* LEAP_DEVICE_INFO* FLeapWrapper::GetDeviceProperties()
{
	LEAP_DEVICE_INFO* currentDevice;
	DataLock->Lock();
	currentDevice = CurrentDeviceInfo;
	DataLock->Unlock();
	return currentDevice;
}
*/
const char* FLeapWrapper::ResultString(eLeapRS Result)
{
	switch (Result)
	{
		case eLeapRS_Success:
			return "eLeapRS_Success";
		case eLeapRS_UnknownError:
			return "eLeapRS_UnknownError";
		case eLeapRS_InvalidArgument:
			return "eLeapRS_InvalidArgument";
		case eLeapRS_InsufficientResources:
			return "eLeapRS_InsufficientResources";
		case eLeapRS_InsufficientBuffer:
			return "eLeapRS_InsufficientBuffer";
		case eLeapRS_Timeout:
			return "eLeapRS_Timeout";
		case eLeapRS_NotConnected:
			return "eLeapRS_NotConnected";
		case eLeapRS_HandshakeIncomplete:
			return "eLeapRS_HandshakeIncomplete";
		case eLeapRS_BufferSizeOverflow:
			return "eLeapRS_BufferSizeOverflow";
		case eLeapRS_ProtocolError:
			return "eLeapRS_ProtocolError";
		case eLeapRS_InvalidClientID:
			return "eLeapRS_InvalidClientID";
		case eLeapRS_UnexpectedClosed:
			return "eLeapRS_UnexpectedClosed";
		case eLeapRS_UnknownImageFrameRequest:
			return "eLeapRS_UnknownImageFrameRequest";
		case eLeapRS_UnknownTrackingFrameID:
			return "eLeapRS_UnknownTrackingFrameID";
		case eLeapRS_RoutineIsNotSeer:
			return "eLeapRS_RoutineIsNotSeer";
		case eLeapRS_TimestampTooEarly:
			return "eLeapRS_TimestampTooEarly";
		case eLeapRS_ConcurrentPoll:
			return "eLeapRS_ConcurrentPoll";
		case eLeapRS_NotAvailable:
			return "eLeapRS_NotAvailable";
		case eLeapRS_NotStreaming:
			return "eLeapRS_NotStreaming";
		case eLeapRS_CannotOpenDevice:
			return "eLeapRS_CannotOpenDevice";
		default:
			return "unknown result type.";
	}
}

void FLeapWrapper::EnableImageStream(bool bEnable)
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

void FLeapWrapper::Millisleep(int milliseconds)
{
	FPlatformProcess::Sleep(((float) milliseconds) / 1000.f);
}


void FLeapWrapper::SetFrame(const LEAP_TRACKING_EVENT* Frame)
{
	DataLock->Lock();

	if (!LatestFrame)
	{
		LatestFrame = (LEAP_TRACKING_EVENT*) malloc(sizeof(*Frame));
	}

	*LatestFrame = *Frame;

	DataLock->Unlock();
}

/** Called by ServiceMessageLoop() when a connection event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleConnectionEvent(const LEAP_CONNECTION_EVENT* ConnectionEvent)
{
	bIsConnected = true;
	if (ConnectorCallbackDelegate)
	{
		ConnectorCallbackDelegate->OnConnect();
	}
}

/** Called by ServiceMessageLoop() when a connection lost event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT* ConnectionLostEvent)
{
	bIsConnected = false;
	

	if (ConnectorCallbackDelegate)
	{
		ConnectorCallbackDelegate->OnConnectionLost();
	}
}

/**
 * Called by ServiceMessageLoop() when a device event is returned by LeapPollConnection()
 */
void FLeapWrapper::HandleDeviceEvent(const LEAP_DEVICE_EVENT* DeviceEvent)
{
	LEAP_DEVICE DeviceHandle = nullptr;
	// Open device using LEAP_DEVICE_REF from event struct.
	eLeapRS Result = LeapOpenDevice(DeviceEvent->device, &DeviceHandle);
	if (Result != eLeapRS_Success)
	{
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("Could not open device %s.\n"), ResultString(Result));
		return;
	}
	
	// Create a struct to hold the device properties, we have to provide a buffer for the serial string
	LEAP_DEVICE_INFO DeviceProperties = {sizeof(DeviceProperties)};
	// Start with a length of 1 (pretending we don't know a priori what the length is).
	// Currently device serial numbers are all the same length, but that could change in the future
	DeviceProperties.serial_length = 64;
	DeviceProperties.serial = (char*) malloc(DeviceProperties.serial_length);
	// This will fail since the serial buffer is only 1 character long
	// But deviceProperties is updated to contain the required buffer length
	Result = LeapGetDeviceInfo(DeviceHandle, &DeviceProperties);
	if (Result == eLeapRS_InsufficientBuffer)
	{
		// try again with correct buffer size
		free(DeviceProperties.serial);
		DeviceProperties.serial = (char*) malloc(DeviceProperties.serial_length);
		Result = LeapGetDeviceInfo(DeviceHandle, &DeviceProperties);
		if (Result != eLeapRS_Success)
		{
			printf("Failed to get device info %s.\n", ResultString(Result));
			free(DeviceProperties.serial);
			return;
		}
	}
	AddDevice(DeviceEvent->device.id, DeviceProperties, DeviceHandle);
	
	if (ConnectorCallbackDelegate)
	{
		TaskRefDeviceFound = FLeapAsync::RunShortLambdaOnGameThread([DeviceEvent, DeviceProperties, this] {
				if (ConnectorCallbackDelegate)
			{
				ConnectorCallbackDelegate->OnDeviceFound(&DeviceProperties);
				free(DeviceProperties.serial);
			}
		});
	}
	else
	{
		free(DeviceProperties.serial);
	}

	LeapCloseDevice(DeviceHandle);
}

/** Called by ServiceMessageLoop() when a device lost event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleDeviceLostEvent(const LEAP_DEVICE_EVENT* DeviceEvent)
{
	if (ConnectorCallbackDelegate)
	{
		TaskRefDeviceLost = FLeapAsync::RunShortLambdaOnGameThread([DeviceEvent, this] {
				if (ConnectorCallbackDelegate)
			{
					FString DeviceSerial;
					for (auto LeapDeviceWrapper : Devices)
					{
						if (LeapDeviceWrapper->GetDeviceID() == DeviceEvent->device.id)
						{
							DeviceSerial = LeapDeviceWrapper->GetDeviceSerial();
							break;
						}
					}
					ConnectorCallbackDelegate->OnDeviceLost(TCHAR_TO_ANSI(*DeviceSerial));
			}
		});
	}
	//TODO: Why does the old code close the device handle once opened?
	RemoveDevice(DeviceEvent->device.id);
}
void FLeapWrapper::AddDevice(const uint32_t DeviceID, const LEAP_DEVICE_INFO& DeviceInfo, const LEAP_DEVICE DeviceHandle)
{
	AsyncTask(ENamedThreads::GameThread,
		[this,DeviceInfo, DeviceID, DeviceHandle]()
		{
			IHandTrackingWrapper* Device = new FLeapDeviceWrapper(DeviceID, DeviceInfo, DeviceHandle, ConnectionHandle, this);
		
			Devices.Add(Device);
			auto Result = LeapSubscribeEvents(ConnectionHandle, DeviceHandle);
			MapDeviceIDToDevice.Add(DeviceID, DeviceHandle);

			NotifyDeviceAdded(Device);
			UE_LOG(
				UltraleapTrackingLog, Log, TEXT("Add Device %s %d."), *(Device->GetDeviceSerial().Right(4)), Device->GetDeviceID());
		
			UE_LOG(UltraleapTrackingLog, Log, TEXT("Device Count %d."), Devices.Num());
		
		});
}
void FLeapWrapper::RemoveDevice(const uint32_t DeviceID)
{
	if (IsInGameThread())
	{
		return RemoveDeviceDirect(DeviceID);
	}
	AsyncTask(ENamedThreads::GameThread,
		[this, DeviceID]() { 
			RemoveDeviceDirect(DeviceID);
		});
}
void FLeapWrapper::RemoveDeviceDirect(const uint32_t DeviceID)
{
	MapDeviceToCallback.Remove(DeviceID);
	MapDeviceIDToDevice.Remove(DeviceID);

	for (auto LeapDeviceWrapper : Devices)
	{
		if (LeapDeviceWrapper->GetDeviceID() == DeviceID)
		{
			UE_LOG(UltraleapTrackingLog, Log, TEXT("Remove Device %s %d."), *(LeapDeviceWrapper->GetDeviceSerial().Right(4)),
				LeapDeviceWrapper->GetDeviceID());

			Devices.Remove(LeapDeviceWrapper);
			CleanupCombinedDevicesReferencingDevice(LeapDeviceWrapper);
			NotifyDeviceRemoved(LeapDeviceWrapper);
			DevicesToCleanup.Remove(LeapDeviceWrapper);
			delete LeapDeviceWrapper;
			break;
		}
	}
	UE_LOG(UltraleapTrackingLog, Log, TEXT("Device Count %d."), Devices.Num());
}
/** Called by ServiceMessageLoop() when a device failure event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT* DeviceFailureEvent, const uint32_t DeviceID)
{
	LeapWrapperCallbackInterface* CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	
	if (CallbackDelegate)
	{
		TaskRefDeviceFailure = FLeapAsync::RunShortLambdaOnGameThread([CallbackDelegate, DeviceFailureEvent, this]
		{
			CallbackDelegate->OnDeviceFailure(DeviceFailureEvent->status, DeviceFailureEvent->hDevice);
		});
	}
}

/** Called by ServiceMessageLoop() when a tracking event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleTrackingEvent(const LEAP_TRACKING_EVENT* TrackingEvent,const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	// Callback delegate is checked twice since the second call happens on the second thread and may be invalidated!
	if (CallbackDelegate)
	{
		// Run this on bg thread still
		CallbackDelegate->OnFrame(TrackingEvent);
	}
}

void FLeapWrapper::HandleImageEvent(const LEAP_IMAGE_EVENT* ImageEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	// Callback with data
	if (CallbackDelegate)
	{
		// Do image handling on background thread for performance
		CallbackDelegate->OnImage(ImageEvent);
	}
}

/** Called by ServiceMessageLoop() when a log event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleLogEvent(const LEAP_LOG_EVENT* LogEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	if (CallbackDelegate)
	{
		TaskRefLog = FLeapAsync::RunShortLambdaOnGameThread(
			[CallbackDelegate, LogEvent, this]
			{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnLog(LogEvent->severity, LogEvent->timestamp, LogEvent->message);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a policy event is returned by LeapPollConnection(). */
void FLeapWrapper::HandlePolicyEvent(const LEAP_POLICY_EVENT* PolicyEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	if (CallbackDelegate)
	{
		// this is always coming back as 0, this means either the Leap service refused to set any flags?
		// or there's a bug in the policy notification system with Leap Motion V4.
		const uint32_t CurrentPolicy = PolicyEvent->current_policy;
		TaskRefPolicy = FLeapAsync::RunShortLambdaOnGameThread(
			[CallbackDelegate, CurrentPolicy, this]
			{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnPolicy(CurrentPolicy);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a policy event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleTrackingModeEvent(const LEAP_TRACKING_MODE_EVENT* TrackingModeEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	if (CallbackDelegate)
	{
		// this is always coming back as 0, this means either the Leap service refused to set any flags?
		// or there's a bug in the policy notification system with Leap Motion V4.
		const uint32_t CurrentMode = TrackingModeEvent->current_tracking_mode;
		TaskRefPolicy = FLeapAsync::RunShortLambdaOnGameThread(
			[CallbackDelegate, CurrentMode, this]
			{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnTrackingMode((eLeapTrackingMode) CurrentMode);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a config change event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT* ConfigChangeEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	if (CallbackDelegate)
	{
		TaskRefConfigChange = FLeapAsync::RunShortLambdaOnGameThread(
			[CallbackDelegate, ConfigChangeEvent, this]
			{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnConfigChange(ConfigChangeEvent->requestID, ConfigChangeEvent->status);
			}
		});
	}
}

/** Called by ServiceMessageLoop() when a config response event is returned by LeapPollConnection(). */
void FLeapWrapper::HandleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT* ConfigResponseEvent, const uint32_t DeviceID)
{
	auto CallbackDelegate = GetCallbackDelegateFromDeviceID(DeviceID);
	if (CallbackDelegate)
	{
		TaskRefConfigResponse = FLeapAsync::RunShortLambdaOnGameThread(
			[CallbackDelegate, ConfigResponseEvent, this]
			{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnConfigResponse(ConfigResponseEvent->requestID, ConfigResponseEvent->value);
			}
		});
	}
}

/**
 * Services the LeapC message pump by calling LeapPollConnection().
 * The average polling time is determined by the framerate of the Leap Motion service.
 */
void FLeapWrapper::ServiceMessageLoop(void* Unused)
{
	eLeapRS Result;
	LEAP_CONNECTION_MESSAGE Msg;
	LEAP_CONNECTION Handle = ConnectionHandle;	  // copy handle so it doesn't get released from under us on game thread

	unsigned int Timeout = 200;
	while (bIsRunning)
	{
		Result = LeapPollConnection(Handle, Timeout, &Msg);

		// Polling may have taken some time, re-check exit condition
		if (!bIsRunning)
		{
			break;
		}

		if (Result != eLeapRS_Success)
		{
			// UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapC PollConnection unsuccessful result %s.\n"),
			// UTF8_TO_TCHAR(ResultString(result)));
			if (!bIsConnected)
			{
				FPlatformProcess::Sleep(0.1f);
				continue;
			}
			else
			{
				continue;
			}
		}

		switch (Msg.type)
		{
			case eLeapEventType_Connection:
				HandleConnectionEvent(Msg.connection_event);
				break;
			case eLeapEventType_ConnectionLost:
				HandleConnectionLostEvent(Msg.connection_lost_event);
				break;
			case eLeapEventType_Device:
				HandleDeviceEvent(Msg.device_event);
				break;
			case eLeapEventType_DeviceLost:
				HandleDeviceLostEvent(Msg.device_event);
				break;
			case eLeapEventType_DeviceFailure:
				HandleDeviceFailureEvent(Msg.device_failure_event, Msg.device_id);
				break;
			case eLeapEventType_Tracking:
				HandleTrackingEvent(Msg.tracking_event, Msg.device_id);
				break;
			case eLeapEventType_Image:
				HandleImageEvent(Msg.image_event, Msg.device_id);
				break;
			case eLeapEventType_LogEvent:
				HandleLogEvent(Msg.log_event, Msg.device_id);
				break;
			case eLeapEventType_Policy:
				HandlePolicyEvent(Msg.policy_event, Msg.device_id);
				break;
			case eLeapEventType_TrackingMode:
				HandleTrackingModeEvent(Msg.tracking_mode_event, Msg.device_id);
				break;
			case eLeapEventType_ConfigChange:
				HandleConfigChangeEvent(Msg.config_change_event, Msg.device_id);
				break;
			case eLeapEventType_ConfigResponse:
				HandleConfigResponseEvent(Msg.config_response_event, Msg.device_id);
				break;
			default:
				// discard unknown message types
				// UE_LOG(UltraleapTrackingLog, Log, TEXT("Unhandled message type %i."), (int32)Msg.type);
				break;
		}	 // switch on msg.type
	}		 // end while running
}
void FLeapWrapper::GetDeviceSerials(TArray<FString>& DeviceSerials)
{
	for (auto Device : Devices)
	{
		DeviceSerials.Add(Device->GetDeviceSerial());
	}
}
IHandTrackingWrapper* FLeapWrapper::FindAggregator(
	const TArray<FString>& DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass)
{
	IHandTrackingWrapper* Ret = nullptr;
	for (auto Combiner : CombinedDevices)
	{
		if (Combiner->MatchDevices(DeviceSerials, DeviceCombinerClass))
		{
			return Combiner;
		}
	}
	return Ret;
}
IHandTrackingWrapper* FLeapWrapper::CreateAggregator(
	const TArray<FString>& DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass)
{
	// use existing if already there
	IHandTrackingWrapper* Ret = FindAggregator(DeviceSerials, DeviceCombinerClass);

	if (Ret)
	{
		return Ret;
	}
	TArray<IHandTrackingWrapper*> DevicesToCombine;
	for (auto DeviceSerial : DeviceSerials)
	{
		auto DeviceWrapper = GetSingularDeviceBySerial(DeviceSerial);
		if (DeviceWrapper)
		{
			DevicesToCombine.Add(DeviceWrapper);
		}
	}
	Ret = new FDeviceCombiner(ConnectionHandle, this, DevicesToCombine, DeviceCombinerClass);
	if (Ret)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Created new aggregator"));
		CombinedDevices.Add(Ret);
		//NotifyDeviceAdded(Ret);
	}
	return Ret;
}
// gets a singular device from the real devices
IHandTrackingWrapper* FLeapWrapper::GetSingularDeviceBySerial(const FString& DeviceSerial)
{
	for (auto Device : Devices)
	{
		if (Device->GetDeviceSerial() == DeviceSerial)
		{
			return Device;
		}
	}
	return nullptr;
}
// gets a device, finds or creates combined device
IHandTrackingWrapper* FLeapWrapper::GetDevice(
	const TArray<FString>& DeviceSerials, const ELeapDeviceCombinerClass DeviceCombinerClass, const bool AllowOpenXR)
{
	IHandTrackingWrapper* Ret = nullptr;
	if (DeviceSerials.Num() == 0 && Devices.Num())
	{
		// fallback device is the first non OpenXR device
		// as OpenXR devices are created at startup these are the first ones in the list
		for (auto Device : Devices)
		{
			if (Device->GetDeviceType() == IHandTrackingWrapper::DEVICE_TYPE_OPENXR && !AllowOpenXR)
			{
				continue;
			}
			else if (AllowOpenXR && Device->GetDeviceType() != IHandTrackingWrapper::DEVICE_TYPE_OPENXR)
			{
				continue;
			}
			return Device;
		}
		// if none found specific to the OpenXR flag, just return the zeroth device
		return Devices[0];
	}


	// singular mode, find the device
	if (DeviceSerials.Num() == 1)
	{
		for (auto Device : Devices)
		{
			if (DeviceSerials.Contains(Device->GetDeviceSerial()))
			{
				Ret = Device;
				break;
			}
		}
	}
	// multi mode, create/find aggregator/combiner
	else if (DeviceSerials.Num() > 1)
	{
		return CreateAggregator(DeviceSerials, DeviceCombinerClass);
	}
	return Ret;
}
void FLeapWrapper::TickDevices(const float DeltaTime) 
{
	// safe point to cleanup force deleted devices
	for (auto DeviceToRemove : DevicesToCleanup)
	{
		RemoveDevice(DeviceToRemove->GetDeviceID());
	}
	DevicesToCleanup.Empty();
	TArray<IHandTrackingWrapper*> AllDevices;

	// tick real devices first
	AllDevices.Append(Devices);
	
	//UE_LOG(UltraleapTrackingLog, Log, TEXT("Device Count %d"), Devices.Num());
	
	AllDevices.Append(CombinedDevices);
	for (auto Device : AllDevices)
	{
		auto InternalDevice = Device->GetDevice();
		if (InternalDevice)
		{
			InternalDevice->Tick(DeltaTime);
		}
	}
}
void FLeapWrapper::TickSendControllerEventsOnDevices()
{
	TArray<IHandTrackingWrapper*> AllDevices;

	// tick real devices first
	AllDevices.Append(Devices);
	AllDevices.Append(CombinedDevices);

	for (auto Device : AllDevices)
	{
		auto InternalDevice = Device->GetDevice();
		if (InternalDevice)
		{
			InternalDevice->SendControllerEvents();
		}
	}
}
ELeapDeviceType FLeapWrapper::GetDeviceTypeFromSerial(const FString& DeviceSerial)
{
	auto Device = GetSingularDeviceBySerial(DeviceSerial);
	if (Device)
	{
		return Device->GetDevice()->GetDeviceType();
	}
	return ELeapDeviceType::LEAP_DEVICE_INVALID;
}
// custom callback system as event delegates don't work in editor
// due to a filter for callineditor deep in UObject
void FLeapWrapper::AddLeapConnectorCallback(ILeapConnectorCallbacks* Callback)
{
	LeapConnectorCallbacks.AddUnique(Callback);
}
void FLeapWrapper::RemoveLeapConnnectorCallback(ILeapConnectorCallbacks* Callback)
{
	LeapConnectorCallbacks.Remove(Callback);
}
void FLeapWrapper::PostEarlyInit()
{
	if (UseOpenXR)
	{
		AddOpenXRDevice(nullptr);
	}
}
	// Must be called from the game thread
void FLeapWrapper::NotifyDeviceAdded(IHandTrackingWrapper* Device)
{
	for (auto Callback : LeapConnectorCallbacks)
	{
		Callback->OnDeviceAdded(Device);
	}
}
void FLeapWrapper::NotifyDeviceRemoved(IHandTrackingWrapper* Device)
{
	for (auto Callback : LeapConnectorCallbacks)
	{
		Callback->OnDeviceRemoved(Device);
	}
}
void FLeapWrapper::CleanupCombinedDevicesReferencingDevice(IHandTrackingWrapper* Device)
{
	TArray<IHandTrackingWrapper*> CombinedDevicesToCleanup;
	// make sure any aggregated/combined devices that reference the removed device get cleaned up
	for (auto CombinedDevice : CombinedDevices)
	{
		if (CombinedDevice->ContainsDevice(Device))
		{
			CombinedDevicesToCleanup.Add(CombinedDevice);
		}
	}
	for (auto CombinedDevice : CombinedDevicesToCleanup)
	{
		CombinedDevices.Remove(CombinedDevice);
		NotifyDeviceRemoved(CombinedDevice);
		delete CombinedDevice;
	}

}
void FLeapWrapper::CleanupBadDevice(IHandTrackingWrapper* DeviceWrapper)
{
	 DevicesToCleanup.AddUnique(DeviceWrapper);
}
void FLeapWrapper::AddOpenXRDevice(LeapWrapperCallbackInterface* InCallbackDelegate)
{
	IHandTrackingWrapper* Device = new FOpenXRToLeapWrapper();

	// no OpenXR Hand Tracking Plugin or no XR hardware?
	if (!Device->IsConnected())
	{
		delete Device;
		return;
	}
	// if we're replacing the leap connect,
	// callback here so the caller gets a connected notification
	if (InCallbackDelegate)
	{
		InCallbackDelegate->OnConnect();
	}
	Devices.Add(Device);

	NotifyDeviceAdded(Device);
	UE_LOG(
		UltraleapTrackingLog, Log, TEXT("Add OpenXR Device %s %d."), *(Device->GetDeviceSerial()), Device->GetDeviceID());
}
#pragma endregion LeapC Wrapper