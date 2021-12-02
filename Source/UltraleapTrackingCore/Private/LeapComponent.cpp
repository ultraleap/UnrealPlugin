

#include "LeapComponent.h"

#include "IUltraleapTrackingPlugin.h"

ULeapComponent::ULeapComponent(const FObjectInitializer& init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	bAddHmdOrigin = false;
	DeviceId = 1;	 // default to first device
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
}
void ULeapComponent::SetSwizzles(
	ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW)
{
	IUltraleapTrackingPlugin::Get().SetSwizzles(ToX, ToY, ToZ, ToW);
}
void ULeapComponent::UninitializeComponent()
{
	// remove ourselves from the delegates
	IUltraleapTrackingPlugin::Get().RemoveEventDelegate(this);

	Super::UninitializeComponent();
}