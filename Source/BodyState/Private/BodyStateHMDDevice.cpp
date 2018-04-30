#include "BodyStateHMDDevice.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"

FBodyStateHMDDevice::FBodyStateHMDDevice()
{
	HMDDeviceIndex = -1;

	//Default config
	Config.DeviceName = TEXT("HMD");
	Config.InputType = EBodyStateDeviceInputType::EXTERNAL_REFERENCE_INPUT_TYPE;
}

FBodyStateHMDDevice::~FBodyStateHMDDevice()
{

}

void FBodyStateHMDDevice::UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton)
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		FQuat Orientation;
		FVector Position;
		GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, Orientation, Position);
		UBodyStateBone* Head = Skeleton->Head();
		if (!Head->IsTracked())
		{
			Head->SetTrackingConfidenceRecursively(1.f);
			Head->Meta.ParentDistinctMeta = true;
			Head->Meta.TrackingType = Config.DeviceName;
		}

		FTransform HMDTransform = FTransform(Orientation, Position, FVector(1.f));
		Head->BoneData.SetFromTransform(HMDTransform);
	}
}

void FBodyStateHMDDevice::OnDeviceDetach()
{
	HMDDeviceIndex = -1;
}
