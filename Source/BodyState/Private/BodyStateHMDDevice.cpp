#include "BodyStateHMDDevice.h"
#include "IHeadMountedDisplay.h"
#include "Runtime/HeadMountedDisplay/Public/XRMotionControllerBase.h"
#include "IXRTrackingSystem.h"

FBodyStateHMDDevice::FBodyStateHMDDevice()
{
	HMDDeviceIndex = -1;

	//Default config
	Config.DeviceName = TEXT("HMD");
	Config.InputType = EBodyStateDeviceInputType::EXTERNAL_REFERENCE_INPUT_TYPE;
	bShouldTrackMotionControllers = true;
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
			Head->Meta.Confidence = 1.f;
			Head->Meta.ParentDistinctMeta = true;
			Head->Meta.TrackingType = Config.DeviceName;
		}

		FTransform HMDTransform = FTransform(Orientation, Position, FVector(1.f));
		Head->BoneData.SetFromTransform(HMDTransform);

		if (bShouldTrackMotionControllers)
		{
			UBodyStateBone* LeftHand = Skeleton->BoneForEnum(EBodyStateBasicBoneType::BONE_HAND_WRIST_L);
			UBodyStateBone* RightHand = Skeleton->BoneForEnum(EBodyStateBasicBoneType::BONE_HAND_WRIST_R);
			
			if (!LeftHand->IsTracked())
			{
				LeftHand->Meta.Confidence = 0.9f;
				LeftHand->Meta.ParentDistinctMeta = true;
				LeftHand->Meta.TrackingType = Config.DeviceName;
			}
			if (!RightHand->IsTracked())
			{
				RightHand->Meta.Confidence = 0.9f;
				RightHand->Meta.ParentDistinctMeta = true;
				RightHand->Meta.TrackingType = Config.DeviceName;
			}

			//enum motion controllers
			TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());

			FRotator OrientationRot;
			
			for(IMotionController* Controller : MotionControllers)
			{
				TArray<FMotionControllerSource> Sources;
				Controller->EnumerateSources(Sources);

				Controller->GetControllerOrientationAndPosition(0, FXRMotionControllerBase::LeftHandSourceId, OrientationRot, Position, 100.f);
				FTransform HandTransform = FTransform(OrientationRot, Position, FVector(1.f));
				LeftHand->BoneData.SetFromTransform(HandTransform);

				Controller->GetControllerOrientationAndPosition(0, FXRMotionControllerBase::RightHandSourceId, OrientationRot, Position, 100.f);
				HandTransform = FTransform(OrientationRot, Position, FVector(1.f));
				LeftHand->BoneData.SetFromTransform(HandTransform);
			}
		}
	}
}

void FBodyStateHMDDevice::OnDeviceDetach()
{
	HMDDeviceIndex = -1;
}
