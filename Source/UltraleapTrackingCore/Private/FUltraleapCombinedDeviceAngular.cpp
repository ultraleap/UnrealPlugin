/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapCombinedDeviceAngular.h"
 
void FUltraleapCombinedDeviceAngular::CombineFrame(const TArray<FLeapFrameData>& SourceFrames)
{
	if (!SourceFrames.Num())
	{
		return;
	}

	CurrentFrame.Hands.Empty();
	MergeHands(SourceFrames, CurrentFrame.Hands, CurrentFrame.LeftHandVisible, CurrentFrame.RightHandVisible);
}
/*
 * Utility function of running average
 */
float AprxAvg(float Avg, float NewSample)
{
	Avg -= Avg / 2;
	Avg += NewSample / 2;

	return Avg;
}
/*
 * This function returns one set of hands from multiple Leap Providers, weighing their influence using hands' positions relative
 * to devices.
 */
void FUltraleapCombinedDeviceAngular::MergeHands(const TArray<FLeapFrameData>& SourceFrames, TArray<FLeapHandData>& MergedHands, bool& LeftHandVisible, bool& RightHandVisible)
{
	// Sort Left and Right hands (some values may be null since never know how many hands are visible, but we clean it up at the
	// end)
	TArray<const FLeapHandData*> LeftHands;
	TArray<const FLeapHandData*> RightHands;
	


	for (int i = 0; i < SourceFrames.Num(); i++)
	{
		for(auto& TempHand : SourceFrames[i].Hands)
		{
			if (TempHand.HandType == LEAP_HAND_LEFT)
			{
				LeftHands.Add(&TempHand);
			}
			else
			{
				RightHands.Add(&TempHand);
			}
		}
	}

	// combine hands using relative angle between devices:
	FLeapHandData ConfidentLeft;
	FLeapHandData ConfidentRight;

	const bool LeftValid = AngularInterpolate(LeftHands, Cam1Alpha, LeftAngle, ConfidentLeft);
	const bool RightValid = AngularInterpolate(RightHands,Cam2Alpha, RightAngle, ConfidentRight);

	// clean up and return hand arrays with only valid hands
	if (LeftValid)
	{
		MergedHands.Add(ConfidentLeft);
		LeftHandVisible = true;
	}
	if (RightValid)
	{
		MergedHands.Add(ConfidentRight);
		RightHandVisible = true;
	}
}

// static
float FUltraleapCombinedDeviceAngular::AngleSigned(const FVector& V1, const FVector& V2, const FVector& N)
{
	// v1 = average palm position
	// v2 = device midpoint up vector
	// n = device midpoint forward vector

	return FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(N, FVector::CrossProduct(V1, V2)), FVector::DotProduct(V1, V2)));
}
/*
 * Combines list of hands (of one chiarality) into one Leap.Hand, by weighing the relative angle to the devices
 */
bool FUltraleapCombinedDeviceAngular::AngularInterpolate(
	const TArray<const FLeapHandData*>& HandList, float& Alpha, float& Angle, FLeapHandData& MergedHand)
{

// find average palm position since we don't exactly know which provider is closest to reality:
	bool HandInit = false;
	int NumValidHands = 0;
	bool IsLeft = false;
	for(auto Hand : HandList)
	{
		IsLeft = Hand->HandType == LEAP_HAND_LEFT;

		if (Hand->Confidence > 0.98f &&
			Hand->VisibleTime > 0.5f)	// only use hands with high confidence to avoid when hand is barely in view
		{
			if (!HandInit)
			{
				MergedHand = *Hand;
				HandInit = true;
			}
			else
			{
				FVector NH = Hand->Palm.Position;
				FVector TH = MergedHand.Palm.Position;
				MergedHand.Palm.Position = FVector(AprxAvg(NH.X, TH.X), AprxAvg(NH.Y, TH.Y), AprxAvg(NH.Z, TH.Z));
			}
			NumValidHands++;
		}
	}
	FVector TempHandPalmPosition;
	if (HandInit)
	{
		TempHandPalmPosition = MergedHand.Palm.Position;
	}


	if (NumValidHands > 0)
	{
		// calculate angle between midpoint between devices(i.e. providers):
		FVector DevicesMiddle = FVector::ZeroVector;
		FVector DevicesAvgForward = FVector::ZeroVector;
		const int NumProviders = DevicesToCombine.Num();

		for (int i = 0; i < NumProviders; i++)
		{
			FTransform SourceDeviceOrigin = GetSourceDeviceOrigin(i);

			// using Up, forward in Unity (UE Up is Z, leapspace forward is Z)
			DevicesAvgForward += SourceDeviceOrigin.GetRotation().GetUpVector();
			DevicesMiddle += SourceDeviceOrigin.GetLocation();
		}
		DevicesAvgForward = DevicesAvgForward / NumProviders;
		DevicesMiddle = DevicesMiddle / NumProviders;

		MidpointDevices.SetLocation(DevicesMiddle);
		// TODO, double check this re leap space, sets .forward in Unity
		MidpointDevices.SetRotation(DevicesAvgForward.Rotation().Quaternion());

		MidDevicePointPosition = MidpointDevices.GetLocation();

		// TODO, double check this, sets .forward in Unity
		MidDevicePointForward = MidpointDevices.GetRotation().GetUpVector();
		// double check, up in unity, = what in leap space
		MidDevicePointUp = MidpointDevices.GetRotation().GetForwardVector();

		FVector AngleCalculationHandPosition = MergedHand.Palm.Position;

		Angle = AngleSigned(AngleCalculationHandPosition,
			// check re leap space, up then forward in Unity
			MidpointDevices.GetLocation() + MidpointDevices.GetRotation().GetForwardVector(), MidpointDevices.GetRotation().GetUpVector());
		
		Alpha = FMath::Clamp(Angle, -MaxInterpolationAngle / 2, MaxInterpolationAngle / 2);
		Alpha = ((Alpha + (MaxInterpolationAngle / 2)) / (MaxInterpolationAngle));	  // normalize to a 0-1 scale

		// Interpolate using alpha:
		if (NumValidHands > 1)	   // Note: this implementation only works with first 2 hands:
		{
			TArray<FVector> JointsCombined;
			FVector MergedPalmPos;
			FQuat MergedPalmRot;
		
			CreateLinearJointListInterp(*HandList[0], *HandList[1], JointsCombined, Alpha, MergedPalmPos, MergedPalmRot);
			ConvertToWorldSpaceHand(MergedHand, IsLeft, MergedPalmPos, MergedPalmRot, JointsCombined);
		}
	}
	return HandInit;
}

