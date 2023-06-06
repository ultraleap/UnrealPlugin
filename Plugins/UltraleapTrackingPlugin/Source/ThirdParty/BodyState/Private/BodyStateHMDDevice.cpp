/*************************************************************************************************************************************
 *The MIT License(MIT)
 *
 *Copyright(c) 2016 Jan Kaniewski(Getnamo)
 *Modified work Copyright(C) 2019 - 2021 Ultraleap, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 *files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
 *merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions :
 *
 *The above copyright notice and this permission notice shall be included in all copies or
 *substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************************************************************************/

#include "BodyStateHMDDevice.h"

#include "Engine/Engine.h"
#include "Features/IModularFeatures.h"
#include "IXRTrackingSystem.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "XRMotionControllerBase.h"

FBodyStateHMDDevice::FBodyStateHMDDevice()
{
	HMDDeviceIndex = -1;

	// Default config
	Config.DeviceName = TEXT("HMD");
	Config.InputType = EBodyStateDeviceInputType::EXTERNAL_REFERENCE_INPUT_TYPE;
	Config.TrackingTags.Add("Hands");
	Config.TrackingTags.Add("Head");
	bShouldTrackMotionControllers = true;
	MotionControllerInertialConfidence = 0.1f;
	MotionControllerTrackedConfidence = 0.8f;	 // it's not 1.0 to allow leap motion to override it if both are tracked at same
												 // time
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
			Head->Meta.TrackingTags = Config.TrackingTags;
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
				LeftHand->Meta.TrackingTags = Config.TrackingTags;
			}
			if (!RightHand->IsTracked())
			{
				RightHand->Meta.Confidence = 0.f;
				RightHand->Meta.ParentDistinctMeta = true;
				RightHand->Meta.TrackingType = Config.DeviceName;
				RightHand->Meta.TrackingTags = Config.TrackingTags;
			}

			// enum motion controllers
			TArray<IMotionController*> MotionControllers =
				IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(
					IMotionController::GetModularFeatureName());

			FRotator OrientationRot = FRotator(0.f, 0.f, 0.f);
			FTransform HandTransform;
			LeftHand->Meta.Confidence = 0.f;
			RightHand->Meta.Confidence = 0.f;

			for (IMotionController* Controller : MotionControllers)
			{
				// Left Hand
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
					if (Hand->Meta.ParentDistinctMeta == false)
					{
						Hand->Meta.ParentDistinctMeta = true;
						Hand->Meta.TrackingTags = Config.TrackingTags;
					}
					Controller->GetControllerOrientationAndPosition(0, TrackingSource, OrientationRot, Position, 100.f);
					HandTransform = FTransform(OrientationRot, Position, FVector(1.f));
					Hand->BoneData.SetFromTransform(HandTransform);
				}

				// Right Hand
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
					if (Hand->Meta.ParentDistinctMeta == false)
					{
						Hand->Meta.ParentDistinctMeta = true;
						Hand->Meta.TrackingTags = Config.TrackingTags;
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
