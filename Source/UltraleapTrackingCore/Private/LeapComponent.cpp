/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapComponent.h"
#include "Multileap/JointOcclusionActor.h"
#include "Kismet/GameplayStatics.h"
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
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		Connector->AddLeapConnectorCallback(this);
	}
}
ULeapComponent::~ULeapComponent() 
{
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		Connector->RemoveLeapConnnectorCallback(this);
	}
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

void ULeapComponent::GetLatestFrameData(FLeapFrameData& OutData, const bool ApplyDeviceOrigin)
{
	if (CurrentHandTrackingDevice)
	{
		IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->GetLatestFrameData(OutData, ApplyDeviceOrigin);
		}
	}
}
void ULeapComponent::ConnectToInputEvents()
{
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
		CurrentHandTrackingDevice = nullptr;
		
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
		if (ActiveDeviceSerial == NameConstantNone)
		{
			return false;
		}
		CurrentHandTrackingDevice = Connector->GetDevice(DeviceSerials, DeviceCombinerClass, false);
		
		Success = (CurrentHandTrackingDevice != nullptr);

		if (Success)
		{
			IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
			if (Device)
			{
				Device->AddEventDelegate(this);
				TrackingMode = Device->GetOptions().Mode;
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
bool ULeapComponent::IsActiveDevicePluggedIn()
{
	return CurrentHandTrackingDevice != nullptr;
}
void ULeapComponent::GetMultiDeviceDebugInfo(int32& NumLeftTracked, int32& NumRightTracked)
{
	if (MultiDeviceMode == ELeapMultiDeviceMode::LEAP_MULTI_DEVICE_COMBINED && CurrentHandTrackingDevice != nullptr)
	{
		auto Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->GetDebugInfo(NumLeftTracked, NumRightTracked);
		}
	}
}
bool ULeapComponent::GetDeviceOrigin(FTransform& DeviceOrigin)
{
	auto Device = CurrentHandTrackingDevice->GetDevice();
	if (Device)
	{
		DeviceOrigin = Device->GetDeviceOrigin();
		return true;
	}
	else
	{
		DeviceOrigin = FTransform::Identity;
	}
	return false;
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
		// Update the options as different settings are needed for VR vs Desktop
		auto Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->SetOptions(Device->GetOptions());
		}
		TrackingMode = Mode;
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
void ULeapComponent::RefreshDeviceList(const bool NotifyChangeToUI) 
{
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		AvailableDeviceSerials.Empty();
		Connector->GetDeviceSerials(AvailableDeviceSerials);
#if WITH_EDITOR
		if (DetailBuilder && NotifyChangeToUI)
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
IHandTrackingDevice* ULeapComponent::GetCombinedDeviceBySerials(const TArray<FString>& DeviceSerials)
{
	// no combined device as not enough devices asked for
	if (DeviceSerials.Num() < 2)
	{
		return nullptr;
	}
	ILeapConnector* Connector = IUltraleapTrackingPlugin::Get().GetConnector();
	if (Connector)
	{
		auto Wrapper = Connector->GetDevice(DeviceSerials, ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_CONFIDENCE, false);
		if (Wrapper) 
		{
			return Wrapper->GetDevice();
		}
	}
	return nullptr;
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
void ULeapComponent::UpdateDeviceOrigin(const FTransform& DeviceOriginIn)
{
	if (CurrentHandTrackingDevice)
	{
		IHandTrackingDevice* Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Device->SetDeviceOrigin(DeviceOriginIn);
		}
	}

}
void ULeapComponent::SetupMultidevice(
	const TArray<FString>& DeviceSerials, const ELeapMultiDeviceMode MultiDeviceModeIn, const ELeapDeviceCombinerClass CombinerClass)
{
	CombinedDeviceSerials = DeviceSerials;
	MultiDeviceMode = MultiDeviceModeIn;
	DeviceCombinerClass = CombinerClass;
	UpdateMultiDeviceMode(MultiDeviceMode);
}
#if WITH_EDITOR
void ULeapComponent::SetCustomDetailsPanel(IDetailLayoutBuilder* DetailBuilderIn)
{
	if (!DetailBuilderIn)
	{
		DetailBuilder = DetailBuilderIn;
		return;
	}
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
void ULeapComponent::OnDeviceAdded(IHandTrackingWrapper* DeviceWrapper)
{
	if (!IsActiveDevicePluggedIn())
	{
		SubscribeToDevice();
	}
	if (DeviceWrapper->GetDeviceSerial() == ActiveDeviceSerial)
	{
		ConnectToInputEvents();
	}
	else
	{
		RefreshDeviceList(true);
	}
}
void ULeapComponent::OnDeviceRemoved(IHandTrackingWrapper* DeviceWrapper)
{
	if (CurrentHandTrackingDevice == DeviceWrapper)
	{
		UnsubscribeFromCurrentDevice();
		CurrentHandTrackingDevice = nullptr;
	}
	RefreshDeviceList(true);
}
bool ULeapComponent::GetLeapOptions(FLeapOptions& Options)
{
	if (CurrentHandTrackingDevice)
	{
		auto Device = CurrentHandTrackingDevice->GetDevice();
		if (Device)
		{
			Options = Device->GetOptions();
			return true;
		}
	}
	return false;
}

void ULeapComponent::GetHandSize(float& OutHandSize)
{
	FLeapFrameData LeapFrameData;
	GetLatestFrameData(LeapFrameData);
	TArray<FLeapHandData> Hands = LeapFrameData.Hands;
	FLeapHandData HandToScale;
	if (!Hands.Num())
	{
		return;
	}
	if (LeapFrameData.LeftHandVisible || LeapFrameData.RightHandVisible)
	{
		HandToScale = Hands[0];
	}

	float Length = 0.0;
	FLeapDigitData MiddleFinger = HandToScale.Middle;
	TArray<FLeapBoneData> Bones = MiddleFinger.Bones;

	// starting from the palm cause there's no wrist position in the frame
	bool AddedPalmToFirstBone = false;
	for (const FLeapBoneData& Bone : Bones)
	{
		if (!AddedPalmToFirstBone)
		{
			Length += FVector::Dist(HandToScale.Palm.Position, Bone.PrevJoint);
			AddedPalmToFirstBone = true;
		}
		Length += FVector::Dist(Bone.PrevJoint, Bone.NextJoint);
	}
	OutHandSize = Length;
}