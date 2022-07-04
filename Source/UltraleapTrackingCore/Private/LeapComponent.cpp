/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapComponent.h"

#include "IUltraleapTrackingPlugin.h"

const FString ULeapComponent::NameConstantNone = "None";


ULeapComponent::ULeapComponent(const FObjectInitializer& init) : 
	UActorComponent(init), CurrentHandTrackingDevice(nullptr)
{
#if WITH_EDITOR
	DetailBuilder = nullptr;
#endif
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	IsConnectedToInputEvents = false;
	bAddHmdOrigin = false;

	OnLeapDeviceAttached.AddDynamic(this, &ULeapComponent::OnDeviceAddedOrRemoved);
	OnLeapDeviceDetached.AddDynamic(this, &ULeapComponent::OnDeviceAddedOrRemoved);

}
ULeapComponent::~ULeapComponent() 
{
	UnsubscribeFromCurrentDevice();
}
void ULeapComponent::SetShouldAddHmdOrigin(bool& bShouldAdd)
{
	// this needs to propagate to all other components with same id
}

void ULeapComponent::AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible)
{
	if (CurrentHandTrackingDevice)
	{
		IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->AreHandsVisible(LeftIsVisible, RightIsVisible);
		}
		
	}
}

void ULeapComponent::GetLatestFrameData(FLeapFrameData& OutData)
{
	if (CurrentHandTrackingDevice)
	{
		IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->GetLatestFrameData(OutData);
		}
	}
}
void ULeapComponent::ConnectToInputEvents()
{
	// Attach delegate references
	IUltraleapTrackingPlugin::Get().AddEventDelegate(this);
	RefreshDeviceList();

	IsConnectedToInputEvents = true;

	// Subscribe to active device
	UpdateActiveDevice(ActiveDeviceSerial);
}
void ULeapComponent::InitializeComponent()
{
	Super::InitializeComponent();
	ConnectToInputEvents();
	
}
void ULeapComponent::SetSwizzles(
	ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW)
{
	if (CurrentHandTrackingDevice)
	{
		CurrentHandTrackingDevice->SetSwizzles(ToX, ToY, ToZ, ToW);
	}
}
bool ULeapComponent::UpdateMultiDeviceMode(const ELeapMultiDeviceMode DeviceMode)
{
	MultiDeviceMode = DeviceMode;
	SubscribeToDevice();
	return true;
}
bool ULeapComponent::UnsubscribeFromCurrentDevice()
{
	bool Success = false;
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector && CurrentHandTrackingDevice)
	{
		IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->RemoveEventDelegate(this);
			Success = true;
		}
	}

	return Success;
}
bool ULeapComponent::SubscribeToDevice()
{
	bool Success = false;
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		// disconnect previous first
		UnsubscribeFromCurrentDevice();
		TArray<FString> DeviceSerials;
		
		if (MultiDeviceMode == ELeapMultiDeviceMode::LEAP_MULTI_DEVICE_SINGULAR)
		{
			// if uninitialised, fallback to default device for backwards compatibility (Empty List = default)
			if (!ActiveDeviceSerial.IsEmpty() && ActiveDeviceSerial != NameConstantNone)
			{
				DeviceSerials.Add(ActiveDeviceSerial);
			}
		}
		// Combined device mode
		else
		{
			DeviceSerials = CombinedDeviceSerials;
		}
		CurrentHandTrackingDevice = Connector->GetDevice(DeviceSerials);
		Success = (CurrentHandTrackingDevice != nullptr);

		if (Success)
		{
			IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
			if (Device)
			{
				Device->AddEventDelegate(this);
			}
		}
	}
	return Success;
}
bool ULeapComponent::UpdateActiveDevice(const FString& DeviceSerial)
{
	
	// this will already be set if Update came from the UI
	// set here for programatically setting it
	ActiveDeviceSerial = DeviceSerial;
	
	return SubscribeToDevice();
}
void ULeapComponent::UninitializeComponent()
{
	// remove ourselves from the delegates
	IUltraleapTrackingPlugin::Get().RemoveEventDelegate(this);
	UnsubscribeFromCurrentDevice();
	Super::UninitializeComponent();
}
void ULeapComponent::SetTrackingMode(ELeapMode Mode)
{
	if (CurrentHandTrackingDevice)
	{
		eLeapTrackingMode LeapMode = eLeapTrackingMode::eLeapTrackingMode_Desktop;
		switch (Mode)
		{
			case ELeapMode::LEAP_MODE_DESKTOP:
				LeapMode = eLeapTrackingMode::eLeapTrackingMode_Desktop;
				break;
			case ELeapMode::LEAP_MODE_SCREENTOP:
				LeapMode = eLeapTrackingMode::eLeapTrackingMode_ScreenTop;
				break;
			case ELeapMode::LEAP_MODE_VR:
				LeapMode = eLeapTrackingMode::eLeapTrackingMode_HMD;
				break;

		}
		CurrentHandTrackingDevice->SetTrackingMode(LeapMode);
	}
}
void ULeapComponent::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	if (CurrentHandTrackingDevice)
	{
		switch (Flag)
		{
			case LEAP_POLICY_BACKGROUND_FRAMES:
				CurrentHandTrackingDevice->SetPolicyFlagFromBoolean(eLeapPolicyFlag_BackgroundFrames, Enable);
				break;
			case LEAP_POLICY_IMAGES:
				CurrentHandTrackingDevice->SetPolicyFlagFromBoolean(eLeapPolicyFlag_Images, Enable);
				break;
			// legacy 3.0 implementation superseded by SetTrackingMode
			case LEAP_POLICY_OPTIMIZE_HMD:
				CurrentHandTrackingDevice->SetPolicyFlagFromBoolean(eLeapPolicyFlag_OptimizeHMD, Enable);
				break;
			case LEAP_POLICY_ALLOW_PAUSE_RESUME:
				CurrentHandTrackingDevice->SetPolicyFlagFromBoolean(eLeapPolicyFlag_AllowPauseResume, Enable);
				break;
			case LEAP_POLICY_MAP_POINTS:
				CurrentHandTrackingDevice->SetPolicyFlagFromBoolean(eLeapPolicyFlag_MapPoints, Enable);
			default:
				break;
		}
	}
}
#if WITH_EDITOR
// Property notifications
void ULeapComponent::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULeapComponent, ActiveDeviceSerial))
	{
		UpdateActiveDevice(ActiveDeviceSerial);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULeapComponent, MultiDeviceMode))
	{
		UpdateMultiDeviceMode(MultiDeviceMode);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULeapComponent, CombinedDeviceSerials))
	{
		UpdateMultiDeviceMode(MultiDeviceMode);
	}
	Super::PostEditChangeProperty(e);
}
#endif
void ULeapComponent::RefreshDeviceList() 
{
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		AvailableDeviceSerials.Empty();
		Connector->GetDeviceSerials(AvailableDeviceSerials);
#if WITH_EDITOR
		if (DetailBuilder)
		{
			auto DeviceSerialProperty = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULeapComponent, ActiveDeviceSerial));
			if (DeviceSerialProperty->IsValidHandle())
			{
#if ENGINE_MAJOR_VERSION > 4
				DeviceSerialProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
#else
				DeviceSerialProperty->NotifyPostChange();
#endif
			}
		}
#endif
	}
}

void ULeapComponent::OnDeviceAddedOrRemoved(FString DeviceName)
{
	RefreshDeviceList();
}
ELeapDeviceType ULeapComponent::GetDeviceTypeFromSerial(const FString& DeviceSerial)
{
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		auto Ret = Connector->GetDeviceTypeFromSerial(DeviceSerial);
		return Ret;
	}
	return ELeapDeviceType::LEAP_DEVICE_INVALID;
}
#if WITH_EDITOR
void ULeapComponent::SetCustomDetailsPanel(IDetailLayoutBuilder* DetailBuilderIn)
{
	if (!IsConnectedToInputEvents || DetailBuilder != DetailBuilderIn)
	{
		DetailBuilder = DetailBuilderIn;
		// connect also populates the device lists
		ConnectToInputEvents();
	}
	else
	{
		RefreshDeviceList();
	}
}
#endif