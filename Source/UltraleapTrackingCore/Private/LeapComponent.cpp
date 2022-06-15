/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapComponent.h"

#include "IUltraleapTrackingPlugin.h"

ULeapComponent::ULeapComponent(const FObjectInitializer& init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	bAddHmdOrigin = false;
	DeviceId = 1;	 // default to first device

	CreateTestState();

	OnLeapDeviceAttached.AddDynamic(this, &ULeapComponent::OnDeviceAddedOrRemoved);
	OnLeapDeviceDetached.AddDynamic(this, &ULeapComponent::OnDeviceAddedOrRemoved);

}
void ULeapComponent::CreateTestState()
{

	AvailableDeviceSerials.Add("John");
	AvailableDeviceSerials.Add("Paul");
	AvailableDeviceSerials.Add("Ringo");

	ActiveDeviceSerial = "Paul";
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

void ULeapComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Attach delegate references
	IUltraleapTrackingPlugin::Get().AddEventDelegate(this);
	RefreshDeviceList();
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
bool ULeapComponent::UpdateActiveDevice(const FString& DeviceSerial)
{
	// this will already be set if Update came from the UI
	// set here for programatically setting it
	ActiveDeviceSerial = DeviceSerial;
	return false;
}
void ULeapComponent::UninitializeComponent()
{
	// remove ourselves from the delegates
	IUltraleapTrackingPlugin::Get().RemoveEventDelegate(this);

	Super::UninitializeComponent();
}
#if WITH_EDITOR
// Property notifications
void ULeapComponent::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == "ActiveDeviceSerial")
	{
		UpdateActiveDevice(ActiveDeviceSerial);
	}
	else if (PropertyName == "MultiDeviceMode")
	{
		UpdateMultiDeviceMode(MultiDeviceMode);
	}
	Super::PostEditChangeProperty(e);
}
void ULeapComponent::RefreshDeviceList() 
{
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		AvailableDeviceSerials.Empty();
		Connector->GetDeviceSerials(AvailableDeviceSerials);
	}
}

void ULeapComponent::OnDeviceAddedOrRemoved(FString DeviceName)
{
	RefreshDeviceList();
	/*auto Property = ULeapComponent::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(ULeapComponent,
	ActiveDeviceSerial));
	 FPropertyChangedEvent DeviceListChanged(
		ULeapComponent::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(ULeapComponent, ActiveDeviceSerial)));
	PostEditChangeProperty(DeviceListChanged);
	ForceRefreshDetails();
	Modify()
	
	PostEditChange();*/
}
#endif