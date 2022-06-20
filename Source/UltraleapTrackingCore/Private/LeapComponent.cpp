/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapComponent.h"

#include "IUltraleapTrackingPlugin.h"

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
	IUltraleapTrackingPlugin::Get().AreHandsVisible(LeftIsVisible, RightIsVisible);
}

void ULeapComponent::GetLatestFrameData(FLeapFrameData& OutData)
{
	IUltraleapTrackingPlugin::Get().GetLatestFrameData(OutData);
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
	IUltraleapTrackingPlugin::Get().SetSwizzles(ToX, ToY, ToZ, ToW);
}
bool ULeapComponent::UpdateMultiDeviceMode(const ELeapMultiDeviceMode DeviceMode)
{
	MultiDeviceMode = DeviceMode;
	return false;
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
		// when combined/aggregated, this can contain more serials
		DeviceSerials.Add(ActiveDeviceSerial);
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
				DeviceSerialProperty->NotifyPostChange();
			}
		}
#endif
	}
}

void ULeapComponent::OnDeviceAddedOrRemoved(FString DeviceName)
{
	RefreshDeviceList();
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