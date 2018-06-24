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
	MotionControllerInertialConfidence = 0.1f;
	MotionControllerTrackedConfidence = 0.8f;
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
				LeftHand->Meta.Confidence = 0.f;
				LeftHand->Meta.ParentDistinctMeta = true;
				LeftHand->Meta.TrackingType = Config.DeviceName;
			}
			if (!RightHand->IsTracked())
			{
				RightHand->Meta.Confidence = 0.f;
				RightHand->Meta.ParentDistinctMeta = true;
				RightHand->Meta.TrackingType = Config.DeviceName;
			}

			//enum motion controllers
			TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());

			FRotator OrientationRot;
			FTransform HandTransform;
			LeftHand->Meta.Confidence = 0.f;
			RightHand->Meta.Confidence = 0.f;
			
			for(IMotionController* Controller : MotionControllers)
			{
				//Left Hand
				UBodyStateBone* Hand = LeftHand;
				FName TrackingSource = FXRMotionControllerBase::LeftHandSourceId;

				ETrackingStatus TrackingStatus = Controller->GetControllerTrackingStatus(0, TrackingSource);
				if (TrackingStatus != ETrackingStatus::NotTracked)
				{
					if (TrackingStatus == ETrackingStatus::Tracked)
					{
						Hand->Meta.Confidence = MotionControllerTrackedConfidence;
					}
					else
					{
						Hand->Meta.Confidence = MotionControllerInertialConfidence;
					}
					Controller->GetControllerOrientationAndPosition(0, TrackingSource, OrientationRot, Position, 100.f);
					HandTransform = FTransform(OrientationRot, Position, FVector(1.f));
					Hand->BoneData.SetFromTransform(HandTransform);
				}

				//Right Hand
				Hand = RightHand;
				TrackingSource = FXRMotionControllerBase::RightHandSourceId;
				
				TrackingStatus = Controller->GetControllerTrackingStatus(0, TrackingSource);
				if (TrackingStatus != ETrackingStatus::NotTracked)
				{
					if (TrackingStatus == ETrackingStatus::Tracked)
					{
						Hand->Meta.Confidence = MotionControllerTrackedConfidence;
					}
					else
					{
						Hand->Meta.Confidence = MotionControllerInertialConfidence;
					}
					Controller->GetControllerOrientationAndPosition(0, TrackingSource, OrientationRot, Position, 100.f);
					HandTransform = FTransform(OrientationRot, Position, FVector(1.f));
					Hand->BoneData.SetFromTransform(HandTransform);
				}
			}
		}
	}
}

void FBodyStateHMDDevice::OnDeviceDetach()
{
	HMDDeviceIndex = -1;
}
