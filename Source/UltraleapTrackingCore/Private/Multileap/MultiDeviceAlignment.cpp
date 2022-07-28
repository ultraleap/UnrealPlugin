/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "MultiDeviceAlignment.h"
#include "LeapComponent.h"
#include "FUltraleapDevice.h"
// Sets default values for this component's properties
UMultiDeviceAlignment::UMultiDeviceAlignment()
{
	AlignmentVariance = 2;
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

FVector CalcCentre(const FVector& PrevJoint, const FVector& NextJoint)
{
	return FMath::Lerp(PrevJoint, NextJoint, 0.5);
}
// Bodystate data is X Up, Y Right, Z Forward
// UE is X Forward, Y Right, Z Up
FTransform ConvertBSToUETransform(const FTransform& TransformLeap)
{
	/* FTransform Ret = TransformLeap;
	Ret.SetLocation(
		FVector(TransformLeap.GetLocation().Z, 
			TransformLeap.GetLocation().Y, 
			TransformLeap.GetLocation().X));

	const float Y = TransformLeap.GetRotation().Rotator().Yaw;
	const float R = TransformLeap.GetRotation().Rotator().Roll;
	const float P = TransformLeap.GetRotation().Rotator().Pitch;

	Ret.SetRotation(FRotator(Y ,R ,P ).Quaternion());*/
	// TODO: now we're inversing to set the device origin, this needs a different function in the other direction
	FTransform Ret = FUltraleapDevice::ConvertUEDeviceOriginToBSTransform(TransformLeap, false);
	return Ret;
}
/* Matrix4x4 newTransform = transform * thisTransform.localToWorldMatrix;
thisTransform.position = newTransform.GetVector3();
thisTransform.rotation = newTransform.GetQuaternion();
thisTransform.localScale = Vector3.Scale(thisTransform.localScale, transform.lossyScale);
*/
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
//	if (!PositioningComplete)
	{
		FLeapFrameData SourceFrame;
		FLeapFrameData TargetFrame;

		FLeapFrameData SourceFrameRaw;
		FLeapFrameData TargetFrameRaw;


		SourceDevice->LeapComponent->GetLatestFrameData(SourceFrame, true);
		TargetDevice->LeapComponent->GetLatestFrameData(TargetFrame, true);

		SourceDevice->LeapComponent->GetLatestFrameData(SourceFrameRaw, false);
		TargetDevice->LeapComponent->GetLatestFrameData(TargetFrameRaw, false);

		TArray<FVector> SourceHandPoints;
		TArray<FVector> TargetHandPoints;
		TArray<FVector> SourceHandPointsRaw;
		TArray<FVector> TargetHandPointsRaw;
		
#ifdef DEBUG_ALIGNMENT
		if (GEngine)
		{
			FString ToPrint = FString::Printf(TEXT("Num Hands %d %d"), SourceFrame.Hands.Num(), TargetFrame.Hands.Num());

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, ToPrint);
		}	
#endif
		int SourceIndex = 0;
		for (auto& SourceHand : SourceFrame.Hands )
		{
			auto& SourceHandRaw = SourceFrameRaw.Hands[SourceIndex];

			auto TargetHand = GetHandFromFrame(TargetFrame, SourceHand.HandType);
			auto TargetHandRaw = GetHandFromFrame(TargetFrameRaw, SourceHand.HandType);
			
			static const int NumFingers = 5;
			static const int NumJoints = 4;

			if (TargetHand != nullptr && TargetHandRaw  != nullptr)
			{
				for (int j = 0; j < NumFingers; j++)
				{
					for (int k = 0; k < NumJoints; k++)
					{
						SourceHandPoints.Add(CalcCentre(SourceHand.Digits[j].Bones[k].PrevJoint,SourceHand.Digits[j].Bones[k].NextJoint));
						
						TargetHandPoints.Add(
							CalcCentre(TargetHand->Digits[j].Bones[k].PrevJoint, TargetHand->Digits[j].Bones[k].NextJoint));

						SourceHandPointsRaw.Add(
							CalcCentre(SourceHandRaw.Digits[j].Bones[k].PrevJoint, SourceHandRaw.Digits[j].Bones[k].NextJoint));
						
						TargetHandPointsRaw.Add(
							CalcCentre(TargetHandRaw->Digits[j].Bones[k].PrevJoint, TargetHandRaw->Digits[j].Bones[k].NextJoint));
					}
				}

				// This is temporary while we check if any of the hands points are not close enough to each other
				PositioningComplete = true;

				for (int i = 0; i < SourceHandPointsRaw.Num(); i++)
				{
					const auto Distance = FVector::Distance(SourceHandPoints[i], TargetHandPoints[i]);
					const auto DistanceRaw = FVector::Distance(SourceHandPointsRaw[i], TargetHandPointsRaw[i]);
					
					if (Distance > AlignmentVariance)
					{
#ifdef DEBUG_ALIGNMENT
						if (GEngine)
						{
							FString ToPrint = FString::Printf(TEXT("Distance %f %f"), Distance, DistanceRaw);

							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, ToPrint);
						}
#endif
						// we are already as aligned as we need to be, we can exit the alignment stage
						PositioningComplete = false;
						break;
					}
				}

				if (PositioningComplete)
				{
				//	return;
				}

				FMatrix DeviceToOriginDeviceMatrix = Solver.SolveKabsch(TargetHandPoints, SourceHandPoints, 200);
				FVector TranslationOnly = Solver.GetTranslation();
				//FTransform ActorTransform =  ConvertLeapToUETransform(TargetDevice->GetActorTransform());
				FTransform ActorTransformFromSolver = FTransform(DeviceToOriginDeviceMatrix);
				TranslationOnly = ActorTransformFromSolver.GetLocation();

				FTransform TransformPositionOnly(FRotator::ZeroRotator, TranslationOnly);
				TransformPositionOnly = ConvertBSToUETransform(TransformPositionOnly);
				ActorTransformFromSolver = ConvertBSToUETransform(ActorTransformFromSolver);
				// to move the target device, we need to be in UE space. This layer is in LeapSpace so convert
				FTransform ActorTransform = TargetDevice->GetActorTransform();

				FRotator OriginalRotation = ActorTransform.GetRotation().Rotator();
				ActorTransform *= TransformPositionOnly;
				
		//		TargetDevice->TeleportTo(ActorTransform.GetLocation(), ActorTransform.GetRotation().Rotator(), false, true);
				TargetDevice->TeleportTo(ActorTransform.GetLocation(),OriginalRotation, false, true);

				return;
			}
		}
	}
}