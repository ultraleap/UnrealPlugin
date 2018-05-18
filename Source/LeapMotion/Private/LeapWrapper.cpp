/******************************************************************************\
* Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include "LeapWrapper.h"
#include "LeapC.h"
#include "LeapLambdaRunnable.h"

#pragma region LeapC Wrapper

//Static callback delegate pointer initialization
LeapWrapperCallbackInterface* FLeapWrapper::CallbackDelegate = nullptr;

FLeapWrapper::FLeapWrapper():
	bIsConnected(false),
	bIsRunning(false)
{
	ProducerLambdaThread = nullptr;
	CallbackDelegate = nullptr;
	interpolatedFrame = nullptr;
	interpolatedFrameSize = 0;
}

FLeapWrapper::~FLeapWrapper()
{
	bIsRunning = false;
	CallbackDelegate = nullptr;
	lastFrame = nullptr;
	if (bIsConnected) 
	{
		CloseConnection();
	}
	if (imageDescription != NULL)
	{
		if(imageDescription->pBuffer != NULL)
		{
			free(imageDescription->pBuffer);
		}
		delete imageDescription;
	}
}

void FLeapWrapper::SetCallbackDelegate(const LeapWrapperCallbackInterface* InCallbackDelegate)
{
	CallbackDelegate = (LeapWrapperCallbackInterface*)InCallbackDelegate;
}

LEAP_CONNECTION* FLeapWrapper::OpenConnection(const LeapWrapperCallbackInterface* InCallbackDelegate)
{
	SetCallbackDelegate(InCallbackDelegate);

	eLeapRS result = LeapCreateConnection(NULL, &connectionHandle);
	if (result == eLeapRS_Success) {
		result = LeapOpenConnection(connectionHandle);
		if (result == eLeapRS_Success) {
			bIsRunning = true;

			LEAP_CONNECTION* Handle = &connectionHandle;
			ProducerLambdaThread = FLeapLambdaRunnable::RunLambdaOnBackGroundThread([&, Handle]
			{
				UE_LOG(LeapMotionLog, Log, TEXT("serviceMessageLoop started."));
				serviceMessageLoop();
				UE_LOG(LeapMotionLog, Log, TEXT("serviceMessageLoop stopped."));

				CloseConnectionHandle(Handle);
			});
		}
	}
	return &connectionHandle;
}

void FLeapWrapper::CloseConnection() 
{
	if (!bIsConnected)
	{
		//Not connected, already done
		UE_LOG(LeapMotionLog, Log, TEXT("Attempt at closing an already closed connection."));
		return;
	}
	bIsConnected = false;
	bIsRunning = false;
	cleanupLastDevice();

	//Wait for thread to exit - Blocking call, but it should be very quick.
	ProducerLambdaThread->WaitForCompletion();
	
	//Nullify the callback delegate. Any outstanding task graphs will not run if the delegate is nullified.
	CallbackDelegate = nullptr;

	UE_LOG(LeapMotionLog, Log, TEXT("Connection successfully closed."));
	//CloseConnectionHandle(&connectionHandle);
}

void FLeapWrapper::SetPolicy(int64 Flags, int64 ClearFlags)
{
	LeapSetPolicyFlags(connectionHandle, Flags, ClearFlags);
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
	LEAP_TRACKING_EVENT *currentFrame;
	dataLock.Lock();
	currentFrame = lastFrame;
	dataLock.Unlock();
	return currentFrame;
}

LEAP_TRACKING_EVENT* FLeapWrapper::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	uint64_t FrameSize = 0;
	LeapGetFrameSize(connectionHandle, TimeStamp, &FrameSize);

	//Check validity of frame size
	if (FrameSize > 0 )
	{
		//Different frame? 
		if (FrameSize != interpolatedFrameSize)
		{
			//If we already have an allocated frame, free it
			if (interpolatedFrame)
			{
				free(interpolatedFrame);
			}
			interpolatedFrame = (LEAP_TRACKING_EVENT *)malloc(FrameSize);
		}
		interpolatedFrameSize = FrameSize;

		//Grab the new frame
		LeapInterpolateFrame(connectionHandle, TimeStamp, interpolatedFrame, interpolatedFrameSize);
	}

	return interpolatedFrame;
}

LEAP_DEVICE_INFO* FLeapWrapper::GetDeviceProperties()
{
	LEAP_DEVICE_INFO *currentDevice;
	dataLock.Lock();
	currentDevice = lastDevice;
	dataLock.Unlock();
	return currentDevice;
}

const char* FLeapWrapper::ResultString(eLeapRS r)
{
	switch (r) {
	case eLeapRS_Success:                  return "eLeapRS_Success";
	case eLeapRS_UnknownError:             return "eLeapRS_UnknownError";
	case eLeapRS_InvalidArgument:          return "eLeapRS_InvalidArgument";
	case eLeapRS_InsufficientResources:    return "eLeapRS_InsufficientResources";
	case eLeapRS_InsufficientBuffer:       return "eLeapRS_InsufficientBuffer";
	case eLeapRS_Timeout:                  return "eLeapRS_Timeout";
	case eLeapRS_NotConnected:             return "eLeapRS_NotConnected";
	case eLeapRS_HandshakeIncomplete:      return "eLeapRS_HandshakeIncomplete";
	case eLeapRS_BufferSizeOverflow:       return "eLeapRS_BufferSizeOverflow";
	case eLeapRS_ProtocolError:            return "eLeapRS_ProtocolError";
	case eLeapRS_InvalidClientID:          return "eLeapRS_InvalidClientID";
	case eLeapRS_UnexpectedClosed:         return "eLeapRS_UnexpectedClosed";
	case eLeapRS_UnknownImageFrameRequest: return "eLeapRS_UnknownImageFrameRequest";
	case eLeapRS_UnknownTrackingFrameID:   return "eLeapRS_UnknownTrackingFrameID";
	case eLeapRS_RoutineIsNotSeer:         return "eLeapRS_RoutineIsNotSeer";
	case eLeapRS_TimestampTooEarly:        return "eLeapRS_TimestampTooEarly";
	case eLeapRS_ConcurrentPoll:           return "eLeapRS_ConcurrentPoll";
	case eLeapRS_NotAvailable:             return "eLeapRS_NotAvailable";
	case eLeapRS_NotStreaming:             return "eLeapRS_NotStreaming";
	case eLeapRS_CannotOpenDevice:         return "eLeapRS_CannotOpenDevice";
	default:                               return "unknown result type.";
	}
}

void FLeapWrapper::EnableImageStream(bool bEnable)
{
	//TODO: test the image/buffer stream code

	if (imageDescription == NULL)
	{
		imageDescription = new LEAP_IMAGE_FRAME_DESCRIPTION;
		imageDescription->pBuffer = NULL;
	}

	int oldLength = imageDescription->buffer_len;

	//if the size is different realloc the buffer
	if (imageDescription->buffer_len != oldLength)
	{
		if (imageDescription->pBuffer != NULL)
		{
			free(imageDescription->pBuffer);
		}
		imageDescription->pBuffer = (void*)malloc(imageDescription->buffer_len);
	}
}


void FLeapWrapper::millisleep(int milliseconds)
{
	FPlatformProcess::Sleep(((float)milliseconds) / 1000.f);
}

void FLeapWrapper::setDevice(const LEAP_DEVICE_INFO *deviceProps)
{
	dataLock.Lock();
	if (lastDevice)
	{
		free(lastDevice->serial);
	}
	else 
	{
		lastDevice = (LEAP_DEVICE_INFO*) malloc(sizeof(*deviceProps));
	}
	*lastDevice = *deviceProps;
	lastDevice->serial = (char*)malloc(deviceProps->serial_length);
	memcpy(lastDevice->serial, deviceProps->serial, deviceProps->serial_length);
	dataLock.Unlock();
}

void FLeapWrapper::cleanupLastDevice()
{
	if (lastDevice)
	{
		free(lastDevice->serial);
	}
	lastDevice = nullptr;
}

void FLeapWrapper::setFrame(const LEAP_TRACKING_EVENT *frame)
{
	dataLock.Lock();
	if (!lastFrame) lastFrame = (LEAP_TRACKING_EVENT *)malloc(sizeof(*frame));
	*lastFrame = *frame;
	dataLock.Unlock();
}


/** Called by serviceMessageLoop() when a connection event is returned by LeapPollConnection(). */
void FLeapWrapper::handleConnectionEvent(const LEAP_CONNECTION_EVENT *connection_event)
{
	bIsConnected = true;
	if (CallbackDelegate) 
	{
		CallbackDelegate->OnConnect();
	}
}

/** Called by serviceMessageLoop() when a connection lost event is returned by LeapPollConnection(). */
void FLeapWrapper::handleConnectionLostEvent(const LEAP_CONNECTION_LOST_EVENT *connection_lost_event)
{
	bIsConnected = false;
	cleanupLastDevice();

	if (CallbackDelegate) 
	{
		CallbackDelegate->OnConnectionLost();
	}
}

/**
* Called by serviceMessageLoop() when a device event is returned by LeapPollConnection()
* Demonstrates how to access device properties.
*/
void FLeapWrapper::handleDeviceEvent(const LEAP_DEVICE_EVENT *device_event)
{
	LEAP_DEVICE deviceHandle;
	//Open device using LEAP_DEVICE_REF from event struct.
	eLeapRS result = LeapOpenDevice(device_event->device, &deviceHandle);
	if (result != eLeapRS_Success)
	{
		UE_LOG(LeapMotionLog, Log, TEXT("Could not open device %s.\n"), ResultString(result));
		return;
	}

	//Create a struct to hold the device properties, we have to provide a buffer for the serial string
	LEAP_DEVICE_INFO deviceProperties = { sizeof(deviceProperties) };
	// Start with a length of 1 (pretending we don't know a priori what the length is).
	// Currently device serial numbers are all the same length, but that could change in the future
	deviceProperties.serial_length = 1;
	deviceProperties.serial = (char*)malloc(deviceProperties.serial_length);
	//This will fail since the serial buffer is only 1 character long
	// But deviceProperties is updated to contain the required buffer length
	result = LeapGetDeviceInfo(deviceHandle, &deviceProperties);
	if (result == eLeapRS_InsufficientBuffer) 
	{
		//try again with correct buffer size
		free(deviceProperties.serial);
		deviceProperties.serial = (char*)malloc(deviceProperties.serial_length);
		result = LeapGetDeviceInfo(deviceHandle, &deviceProperties);
		if (result != eLeapRS_Success) {
			printf("Failed to get device info %s.\n", ResultString(result));
			free(deviceProperties.serial);
			return;
		}
	}
	setDevice(&deviceProperties);
	if (CallbackDelegate) 
	{
		TaskRefDeviceFound = FLeapLambdaRunnable::RunShortLambdaOnGameThread([device_event, deviceProperties, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnDeviceFound(&deviceProperties);
			}
		});
	}

	free(deviceProperties.serial);
	LeapCloseDevice(deviceHandle);
}

/** Called by serviceMessageLoop() when a device lost event is returned by LeapPollConnection(). */
void FLeapWrapper::handleDeviceLostEvent(const LEAP_DEVICE_EVENT *device_event) {
	if (CallbackDelegate)
	{
		TaskRefDeviceLost = FLeapLambdaRunnable::RunShortLambdaOnGameThread([device_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnDeviceLost(lastDevice->serial);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a device failure event is returned by LeapPollConnection(). */
void FLeapWrapper::handleDeviceFailureEvent(const LEAP_DEVICE_FAILURE_EVENT *device_failure_event) 
{
	if (CallbackDelegate) 
	{
		TaskRefDeviceFailure = FLeapLambdaRunnable::RunShortLambdaOnGameThread([device_failure_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnDeviceFailure(device_failure_event->status, device_failure_event->hDevice);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a tracking event is returned by LeapPollConnection(). */
void FLeapWrapper::handleTrackingEvent(const LEAP_TRACKING_EVENT *tracking_event) {
	setFrame(tracking_event); //support polling tracking data from different thread

	//Callback delegate is checked twice since the second call happens on the second thread and may be invalidated!
	if (CallbackDelegate) 
	{
		LeapWrapperCallbackInterface* SafeDelegate = CallbackDelegate;

		//Run this on bg thread still
		CallbackDelegate->OnFrame(tracking_event);
	}
}

void FLeapWrapper::handleImageEvent(const LEAP_IMAGE_EVENT *image_event)
{
	//Todo: handle allocation /etc such that we just have the data ready to push to the end user.
	if (CallbackDelegate)
	{
		TaskRefImageComplete = FLeapLambdaRunnable::RunShortLambdaOnGameThread([image_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnImage(image_event);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a log event is returned by LeapPollConnection(). */
void FLeapWrapper::handleLogEvent(const LEAP_LOG_EVENT *log_event) 
{
	if (CallbackDelegate)
	{
		TaskRefLog = FLeapLambdaRunnable::RunShortLambdaOnGameThread([log_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnLog(log_event->severity, log_event->timestamp, log_event->message);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a policy event is returned by LeapPollConnection(). */
void FLeapWrapper::handlePolicyEvent(const LEAP_POLICY_EVENT *policy_event)
{
	if (CallbackDelegate)
	{
		TaskRefPolicy = FLeapLambdaRunnable::RunShortLambdaOnGameThread([policy_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnPolicy(policy_event->current_policy);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a config change event is returned by LeapPollConnection(). */
void FLeapWrapper::handleConfigChangeEvent(const LEAP_CONFIG_CHANGE_EVENT *config_change_event)
{
	if (CallbackDelegate)
	{
		TaskRefConfigChange = FLeapLambdaRunnable::RunShortLambdaOnGameThread([config_change_event, this]
		{
			if (CallbackDelegate)
			{
				CallbackDelegate->OnConfigChange(config_change_event->requestID, config_change_event->status);
			}
		});
	}
}

/** Called by serviceMessageLoop() when a config response event is returned by LeapPollConnection(). */
void FLeapWrapper::handleConfigResponseEvent(const LEAP_CONFIG_RESPONSE_EVENT *config_response_event)
{
	if (CallbackDelegate)
	{
		TaskRefConfigResponse = FLeapLambdaRunnable::RunShortLambdaOnGameThread([config_response_event, this]
		{
			if (CallbackDelegate) 
			{
				CallbackDelegate->OnConfigResponse(config_response_event->requestID, config_response_event->value);
			}
		});
	}
}

/**
* Services the LeapC message pump by calling LeapPollConnection().
* The average polling time is determined by the framerate of the Leap Motion service.
*/
void FLeapWrapper::serviceMessageLoop(void * unused)
{
	eLeapRS result;
	LEAP_CONNECTION_MESSAGE msg;
	unsigned int timeout = 1000;
	while (bIsRunning) 
	{
		result = LeapPollConnection(connectionHandle, timeout, &msg);

		if (result != eLeapRS_Success)
		{
			//UE_LOG(LeapMotionLog, Log, TEXT("LeapC PollConnection unsuccessful result %s.\n"), UTF8_TO_TCHAR(ResultString(result)));
			if (!bIsConnected)
			{
				FPlatformProcess::Sleep(5.f);
			}
			else
			{
				continue;
			}
		}

		switch (msg.type)
		{
			case eLeapEventType_Connection:
				handleConnectionEvent(msg.connection_event);
				break;
			case eLeapEventType_ConnectionLost:
				handleConnectionLostEvent(msg.connection_lost_event);
				break;
			case eLeapEventType_Device:
				handleDeviceEvent(msg.device_event);
				break;
			case eLeapEventType_DeviceLost:
				handleDeviceLostEvent(msg.device_event);
				break;
			case eLeapEventType_DeviceFailure:
				handleDeviceFailureEvent(msg.device_failure_event);
				break;
			case eLeapEventType_Tracking:
				handleTrackingEvent(msg.tracking_event);
				break;
			case eLeapEventType_Image:
				handleImageEvent(msg.image_event);
			case eLeapEventType_LogEvent:
				handleLogEvent(msg.log_event);
				break;
			case eLeapEventType_Policy:
				handlePolicyEvent(msg.policy_event);
				break;
			case eLeapEventType_ConfigChange:
				handleConfigChangeEvent(msg.config_change_event);
				break;
			case eLeapEventType_ConfigResponse:
				handleConfigResponseEvent(msg.config_response_event);
				break;
			default:
				//discard unknown message types
				//UE_LOG(LeapMotionLog, Log, TEXT("Unhandled message type %i."), (int32)msg.type);
				break;
		} //switch on msg.type
	}//end while running
}

#pragma endregion LeapC Wrapper