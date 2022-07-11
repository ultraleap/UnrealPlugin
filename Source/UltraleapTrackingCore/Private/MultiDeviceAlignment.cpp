/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "MultiDeviceAlignment.h"
#include "LeapComponent.h"

// Sets default values for this component's properties
UMultiDeviceAlignment::UMultiDeviceAlignment()
{
	AlignmentVariance = 0.2;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMultiDeviceAlignment::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrackingDevices();
	
}


// Called every frame
void UMultiDeviceAlignment::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Update();
}
void UMultiDeviceAlignment::UpdateTrackingDevices()
{
	if (SourceDevice && TargetDevice)
	{
		
	}
}
#if WITH_EDITOR
// Property notifications
void UMultiDeviceAlignment::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiDeviceAlignment, SourceDevice))
	{
		UpdateTrackingDevices();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiDeviceAlignment, TargetDevice))
	{
		UpdateTrackingDevices();
	}
	Super::PostEditChangeProperty(e);
}
#endif

void UMultiDeviceAlignment::ReAlignProvider()
{
	if (TargetDevice)
	{
		TargetDevice->SetActorTransform(FTransform(FRotator::ZeroRotator, FVector::ZeroVector));
	}
	PositioningComplete = false;
}
const FLeapHandData* GetHandFromFrame(const FLeapFrameData& Frame, const EHandType HandType)
{
	for (auto& Hand : Frame.Hands)
	{
		if (Hand.HandType == HandType)
		{
			return &Hand;
		}
	}
	return nullptr;
}
// port from LeapUnityExtensions
void Transform(FTransform& ThisTransform, const FMatrix& Transform)
{
	FMatrix NewTransform = Transform * ThisTransform.ToMatrixWithScale();
	ThisTransform.SetLocation(NewTransform.GetOrigin());
	ThisTransform.SetRotation(NewTransform.ToQuat());
	//ThisTransform.SetScale(FVector::Scale(thisTransform.localScale, transform.lossyScale);
}
FVector CalcCentre(const FVector& PrevJoint, const FVector& NextJoint)
{
	return FMath::Lerp(PrevJoint, NextJoint, 0.5);
}
void UMultiDeviceAlignment::Update()
	{
	if (!TargetDevice || !SourceDevice)
	{
		return;
	}
	if (!SourceDevice->LeapComponent->IsActiveDevicePluggedIn() || !TargetDevice->LeapComponent->IsActiveDevicePluggedIn())
	{
		return;
	}
	if (!PositioningComplete)
	{
		FLeapFrameData SourceFrame;
		FLeapFrameData TargetFrame;

		SourceDevice->LeapComponent->GetLatestFrameData(SourceFrame);
		TargetDevice->LeapComponent->GetLatestFrameData(TargetFrame);

		TArray<FVector> SourceHandPoints;
		TArray<FVector> TargetHandPoints;
		
		for (auto& SourceHand : SourceFrame.Hands)
		{
			auto TargetHand = GetHandFromFrame(TargetFrame, SourceHand.HandType);
			
			static const int NumFingers = 5;
			static const int NumJoints = 4;

			if (TargetHand != nullptr)
			{
				for (int j = 0; j < NumFingers; j++)
				{
					for (int k = 0; k < NumJoints; k++)
					{
						SourceHandPoints.Add(CalcCentre(SourceHand.Digits[j].Bones[k].PrevJoint,SourceHand.Digits[j].Bones[k].NextJoint));
						TargetHandPoints.Add(CalcCentre(TargetHand->Digits[j].Bones[k].PrevJoint,TargetHand->Digits[j].Bones[k].NextJoint));
					}
				}

				// This is temporary while we check if any of the hands points are not close enough to eachother
				PositioningComplete = true;

				for (int i = 0; i < SourceHandPoints.Num(); i++)
				{
					if (FVector::Distance(SourceHandPoints[i], TargetHandPoints[i]) > AlignmentVariance)
					{
						// we are already as aligned as we need to be, we can exit the alignment stage
						PositioningComplete = false;
						break;
					}
				}

				if (PositioningComplete)
				{
					return;
				}

				FMatrix DeviceToOriginDeviceMatrix = Solver.SolveKabsch(TargetHandPoints, SourceHandPoints, 200);

				FTransform ActorTransform = TargetDevice->GetActorTransform();
				//Transform(ActorTransform, DeviceToOriginDeviceMatrix);
				// TODO: double check this is the same as the Transform util
				ActorTransform = FTransform(DeviceToOriginDeviceMatrix) * ActorTransform;
				TargetDevice->SetActorTransform(ActorTransform);
				//TransformToOrigin.TransformLocation(ActorTransform.);
					//.transform.Transform(deviceToOriginDeviceMatrix);

				return;
			}
		}
	}
}