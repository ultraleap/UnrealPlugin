/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapCombinedDeviceConfidence.h"
#include "LeapBlueprintFunctionLibrary.h" // for AngleBetweenVectors()

// TODO: pull this in from somewhere
static const int NUM_JOINT_POSITIONS = 21;
// VectorHand.NUM_JOINT_POSITIONS
float GetTime()
{
	return FPlatformTime::Seconds();
}
float SumFloatArray(const TArray<float>& ToSum)
{
	float Ret = 0;
	for (auto Value : ToSum)
	{
		Ret += Value;
	}
	return Ret;
}
float Sum2DFloatArray(const TArray<TArray<float>>& ToSum)
{
	float Ret = 0;
	for (auto ArrayIndex : ToSum)
	{
		for (auto jValue : ArrayIndex)
		{
			Ret += jValue;
		}
		
	}
	// TODO check this is the same
	//LeftJointConfidences.Sum(x = > x[joint_idx] from Unity
	return Ret;
}


void FUltraleapCombinedDeviceConfidence::CombineFrame(const TArray<FLeapFrameData>& SourceFrames)
{
	MergeFrames(SourceFrames, CurrentFrame);
}
// direct port from Unity
void FUltraleapCombinedDeviceConfidence::MergeFrames(const TArray<FLeapFrameData>& SourceFrames, FLeapFrameData& CombinedFrame )
{
	
	const int NumProviders = DevicesToCombine.Num();
	const int NumHandsPerProvider = 2; // until we evolve more

	if (JointConfidences.Num() != (NumProviders * NumHandsPerProvider))
	{
		JointConfidences.AddZeroed(NumProviders * NumHandsPerProvider);
		ConfidencesJointRot.AddZeroed(NumProviders * NumHandsPerProvider);
		ConfidencesJointPalmRot.AddZeroed(NumProviders * NumHandsPerProvider);
		ConfidencesJointOcclusion.AddZeroed(NumProviders * NumHandsPerProvider);
	}
	
	TArray<const FLeapHandData*> LeftHands;
	TArray<const FLeapHandData*> RightHands;

	TArray<float> LeftHandConfidences;
	TArray<float> RightHandConfidences;

	TArray<TArray<float>> LeftJointConfidences;
	TArray<TArray<float>> RightJointConfidences;

	if (JointOcclusionFactor != 0)
	{
		SetupJointOcclusion();
	}

	// make lists of all left and right hands found in each frame and also make a list of their confidences
	for (int FrameIdx = 0; FrameIdx < SourceFrames.Num(); FrameIdx++)
	{
		const FLeapFrameData& Frame = SourceFrames[FrameIdx];
	
		AddFrameToTimeVisibleDicts(SourceFrames, FrameIdx);

		for (const FLeapHandData& Hand : Frame.Hands)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				LeftHands.Add(&Hand);

				float HandConfidence = CalculateHandConfidence(FrameIdx, Hand);
				TArray<float> JointConfidencesLocal;
				CalculateJointConfidence(FrameIdx, Hand, JointConfidencesLocal);

				LeftHandConfidences.Add(HandConfidence);
				LeftJointConfidences.Add(JointConfidencesLocal);
			}
			else
			{
				RightHands.Add(&Hand);

				float HandConfidence = CalculateHandConfidence(FrameIdx, Hand);
				TArray<float> JointConfidencesLocal;
				CalculateJointConfidence(FrameIdx, Hand, JointConfidencesLocal);

				RightHandConfidences.Add(HandConfidence);
				RightJointConfidences.Add(JointConfidencesLocal);
			}
		}
	}

	// normalize hand confidences:
	float Sum = SumFloatArray(LeftHandConfidences);
	
	if (Sum != 0)
	{
		for (int HandsIdx = 0; HandsIdx < LeftHandConfidences.Num(); HandsIdx++)
		{
			LeftHandConfidences[HandsIdx] /= Sum;
		}
	}
	else
	{
		for (int HandsIdx = 0; HandsIdx < LeftHandConfidences.Num(); HandsIdx++)
		{
			LeftHandConfidences[HandsIdx] = 1.0f / LeftHandConfidences.Num();
		}
	}
	Sum = SumFloatArray(RightHandConfidences);
	if (Sum != 0)
	{
		for (int HandsIdx = 0; HandsIdx < RightHandConfidences.Num(); HandsIdx++)
		{
			RightHandConfidences[HandsIdx] /= Sum;
		}
	}
	else
	{
		for (int HandsIdx = 0; HandsIdx < RightHandConfidences.Num(); HandsIdx++)
		{
			RightHandConfidences[HandsIdx] = 1.0f / RightHandConfidences.Num();
		}
	}

	// normalize joint confidences:
	for (int JointIdx = 0; JointIdx < NUM_JOINT_POSITIONS; JointIdx++)
	{
		Sum = Sum2DFloatArray(LeftJointConfidences);

		if (Sum != 0)
		{
			for (int HandsIdx = 0; HandsIdx < LeftJointConfidences.Num(); HandsIdx++)
			{
				LeftJointConfidences[HandsIdx][JointIdx] /= Sum;
			}
		}
		else
		{
			for (int HandsIdx = 0; HandsIdx < LeftJointConfidences.Num(); HandsIdx++)
			{
				LeftJointConfidences[HandsIdx][JointIdx] = 1.0f / LeftJointConfidences.Num();
			}
		}

		Sum = Sum2DFloatArray(RightJointConfidences);
		if (Sum != 0)
		{
			for (int HandsIdx = 0; HandsIdx < RightJointConfidences.Num(); HandsIdx++)
			{
				RightJointConfidences[HandsIdx][JointIdx] /= Sum;
			}
		}
		else
		{
			for (int HandsIdx = 0; HandsIdx < RightJointConfidences.Num(); HandsIdx++)
			{
				RightJointConfidences[HandsIdx][JointIdx] = 1.0f / RightJointConfidences.Num();
			}
		}
	}

	// combine hands using their confidences
	TArray<FLeapHandData> MergedHands;

	if (LeftHands.Num() > 0)
	{
		FLeapHandData Hand;
		
		MergeHands(LeftHands, LeftHandConfidences, LeftJointConfidences, Hand);
		MergedHands.Add(Hand);
	}

	if (RightHands.Num() > 0)
	{
		FLeapHandData Hand;
		MergeHands(RightHands, RightHandConfidences, RightJointConfidences, Hand);
		MergedHands.Add(Hand);
	}

	// create new frame and add merged hands to it
	// TODO: what about the other members of FLeapFrameData?
	CombinedFrame.Hands = MergedHands;
}

/// create joint occlusion gameobjects if they are not there yet and update the position of all joint occlusion gameobjects that are
/// attached to a xr service provider
void FUltraleapCombinedDeviceConfidence::SetupJointOcclusion()
{
	if (JointOcclusions.Num() == 0)
	{
		for(auto Provider : DevicesToCombine)
		{
			//TODO: how to create these in the scene from C++?
			// Ideally pass them down from creation in Blueprint
			/* JointOcclusion JointOcclusion = provider.gameObject.GetComponentInChildren<JointOcclusion>();

			if (JointOcclusion == null)
			{
				JointOcclusion = GameObject.Instantiate(Resources.Load<GameObject>("JointOcclusionPrefab"), provider.transform)
									 .GetComponent<JointOcclusion>();

				foreach(CapsuleHand JointOcclusionHand in JointOcclusion.GetComponentsInChildren<CapsuleHand>(true))
				{
					JointOcclusionHand.leapProvider = Provider;
				}
			}

			jointOcclusions.Add(jointOcclusion);*/
		}

		for(JointOcclusion JointOcclusion : JointOcclusions)
		{
			//jointOcclusion.Setup();
		}
	}
	/*
	// if any providers are xr providers, update their jointOcclusions position and rotation
	for (int i = 0; i < JointOcclusions.Num(); i++)
	{
		LeapXRServiceProvider xrProvider = providers[i] as LeapXRServiceProvider;
		if (xrProvider != null)
		{
			Transform deviceOrigin = GetDeviceOrigin(providers[i]);

			JointOcclusions[i].transform.SetPose(deviceOrigin.GetPose());
			JointOcclusions[i].transform.Rotate(new Vector3(-90, 0, 180));
		}
	}*/
}

/// add all hands in the frame given by frames[frameIdx] to the Dictionaries lastLeftHandPositions and lastRightHandPositions,
/// and update leftHandFirstVisible and rightHandFirstVisible
void FUltraleapCombinedDeviceConfidence::AddFrameToTimeVisibleDicts(const TArray<FLeapFrameData>& Frames,const int FrameIdx)
{
	bool HandsVisible[2] = {false};
	
	for(auto Hand : Frames[FrameIdx].Hands)
	{
		if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
		{
			HandsVisible[0] = true;
			
			if (LeftHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] == 0)
			{
				LeftHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] = GetTime();
			}

			if (!LastLeftHandPositions.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
			{
				LastLeftHandPositions.Add(DevicesToCombine[FrameIdx]->GetDevice(), FHandPositionHistory());
			}
			
			
			LastLeftHandPositions[DevicesToCombine[FrameIdx]->GetDevice()].AddPosition(
				Hand.Palm.Position, GetTime());
		}
		else
		{
			HandsVisible[1] = true;
			if (RightHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] == 0)
			{
				RightHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] = GetTime();
			}

			if (!LastRightHandPositions.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
			{
				LastRightHandPositions.Add(DevicesToCombine[FrameIdx]->GetDevice(), FHandPositionHistory());
			}

			LastRightHandPositions[DevicesToCombine[FrameIdx]->GetDevice()].AddPosition(Hand.Palm.Position, GetTime());
		}
	}

	if (!HandsVisible[0])
	{
		LeftHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] = 0;
	}
	if (!HandsVisible[1])
	{
		RightHandFirstVisible[DevicesToCombine[FrameIdx]->GetDevice()] = 0;
	}
}
// TODO: this will be pulled in from UI components
FTransform GetDeviceOrigin(IHandTrackingDevice* Device)
{
	return FTransform();
}
	/// <summary>
/// combine different confidence functions to get an overall confidence for the given hand
/// uses frame_idx to find the corresponding provider that saw this hand
/// </summary>
float FUltraleapCombinedDeviceConfidence::CalculateHandConfidence(int FrameIdx,const FLeapHandData& Hand)
{
	float Confidence = 0;

	FTransform DeviceOrigin = GetDeviceOrigin(DevicesToCombine[FrameIdx]->GetDevice());

	Confidence = PalmPosFactor *
				 ConfidenceRelativeHandPos(DevicesToCombine[FrameIdx]->GetDevice(), DeviceOrigin, Hand.Palm.Position);
	Confidence +=
		PalmRotFactor * ConfidenceRelativeHandRot(DeviceOrigin, Hand.Palm.Position, Hand.Palm.Normal);
	
	Confidence += PalmVelocityFactor * ConfidenceRelativeHandVelocity(DevicesToCombine[FrameIdx]->GetDevice(), DeviceOrigin,
										   Hand.Palm.Position, Hand.HandType == EHandType::LEAP_HAND_LEFT);

	// if ignoreRecentNewHands is true, then
	// the confidence should be 0 when it is the first frame with the hand in it.
	if (IgnoreRecentNewHands)
	{
		Confidence = Confidence * ConfidenceTimeSinceHandFirstVisible(
									  DevicesToCombine[FrameIdx]->GetDevice(), Hand.HandType == EHandType::LEAP_HAND_LEFT);
	}

	// average out new hand confidence with that of the last few frames
	if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
	{
		if (!HandConfidenceHistoriesLeft.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
		{
			HandConfidenceHistoriesLeft.Add(DevicesToCombine[FrameIdx]->GetDevice(),FHandConfidenceHistory());
		}
		HandConfidenceHistoriesLeft[DevicesToCombine[FrameIdx]->GetDevice()].AddConfidence(Confidence);
		Confidence = HandConfidenceHistoriesLeft[DevicesToCombine[FrameIdx]->GetDevice()].GetAveragedConfidence();
	}
	else
	{
		if (!HandConfidenceHistoriesRight.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
		{
			HandConfidenceHistoriesRight.Add(DevicesToCombine[FrameIdx]->GetDevice(), FHandConfidenceHistory());
		}
		HandConfidenceHistoriesRight[DevicesToCombine[FrameIdx]->GetDevice()].AddConfidence(Confidence);
		Confidence = HandConfidenceHistoriesRight[DevicesToCombine[FrameIdx]->GetDevice()].GetAveragedConfidence();
	}

	return Confidence;
}

/// <summary>
/// uses the hand pos relative to the device to calculate a confidence.
/// using a 2d gauss with bigger spread when further away from the device
/// and amplitude depending on the ideal depth of the specific device and
/// the distance from hand to device
/// </summary>
float FUltraleapCombinedDeviceConfidence::ConfidenceRelativeHandPos(
	IHandTrackingDevice* Provider, const FTransform& DeviceOrigin,const FVector& HandPos)
{
	// TODO could be inversetransform vector, was inversetransform point?
	FVector RelativeHandPos = DeviceOrigin.InverseTransformPosition(HandPos);

	// 2d gauss

	// amplitude
	float a = 1 - RelativeHandPos.Y;
	// pos of center of peak
	float x0 = 0;
	float y0 = 0;
	// spread
	float SigmaX = RelativeHandPos.Y;
	float SigmaY = RelativeHandPos.Y;

	// input pos
	float x = RelativeHandPos.X;
	float y = RelativeHandPos.Z;

	//TODO: there was some logic here to decide if this was a provider/real device
	// presumable to allow you to chain aggregators together?
	// leaving as all IRL devices for the moment, with a special enum for combined LEAP_DEVICE_TYPE_COMBINED
	// if the frame is coming from a LeapServiceProvider, use different values depending on the type of device
	auto DeviceType = Provider->GetDeviceType();
	if (DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_RIGEL || DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_SIR170 ||
		DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_3DI)
	{
		// Depth: Between 10cm to 75cm preferred, up to 1m maximum
		// Field Of View: 170 x 170 degrees typical (160 x 160 degrees minimum)
		// TODO: double check coord system as we're in leap space here vs Unity?
		float CurrentDepth = RelativeHandPos.Y;

		float RequiredWidth = (CurrentDepth / 2.0) / FMath::Sin(FMath::DegreesToRadians(170.0 / 2.0));
		SigmaX = 0.2f * RequiredWidth;
		SigmaY = 0.2f * RequiredWidth;

		// amplitude should be 1 within ideal depth and go 'smoothly' to zero on both sides of the ideal depth.
		if (CurrentDepth > 0.1f && CurrentDepth < 0.75)
		{
			a = 1.0f;
		}
		else if (CurrentDepth < 0.1f)
		{
			a = 0.55f / (PI / 2.0) * FMath::Atan(100 * (CurrentDepth + 0.05f)) + 0.5f;
		}
		else if (CurrentDepth > 0.75f)
		{
			a = -0.55f / (PI / 2.0) * FMath::Atan(50 * (CurrentDepth - 0.875f)) + 0.5f;
		}
	}
	else if (DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_PERIPHERAL)
	{
		// Depth: Between 10cm to 60cm preferred, up to 80cm maximum
		// Field Of View: 140 x 120 degrees typical
		float CurrentDepth = RelativeHandPos.Y;
		float RequiredWidthX = (CurrentDepth / 2) / FMath::Sin(FMath::DegreesToRadians(120.0 / 2.0));
		float RequiredWidthY = (CurrentDepth / 2) / FMath::Sin(FMath::DegreesToRadians(140.0 / 2.0));
		SigmaX = 0.2f * RequiredWidthX;
		SigmaY = 0.2f * RequiredWidthY;

		// amplitude should be 1 within ideal depth and go 'smoothly' to zero on both sides of the ideal depth.
		if (CurrentDepth > 0.1f && CurrentDepth < 0.6f)
		{
			a = 1.0f;
		}
		else if (CurrentDepth < 0.1f)
		{
			a = 0.55f / (PI / 2.0) * FMath::Atan(100 * (CurrentDepth + 0.05f)) + 0.5f;
		}
		else if (CurrentDepth > 0.6f)
		{
			a = -0.55f / (PI / 2.0) * FMath::Atan(50 * (CurrentDepth - 0.7f)) + 0.5f;
		}
	}

	float Confidence =
		a *
		FMath::Exp(-(FMath::Pow(x - x0, 2) / (2 * FMath::Pow(SigmaX, 2)) + FMath::Pow(y - y0, 2) / (2 * FMath::Pow(SigmaY, 2))));

	if (Confidence < 0)
		Confidence = 0;

	return Confidence;
}
/// <summary>
/// uses the palm normal relative to the direction from hand to device to calculate a confidence
/// </summary>
float FUltraleapCombinedDeviceConfidence::ConfidenceRelativeHandRot(
	const FTransform& DeviceOrigin, const FVector& HandPos,const FVector& PalmNormal)
{
	// angle between palm normal and the direction from hand pos to device origin
	float PalmAngle =
		ULeapBlueprintFunctionLibrary::AngleBetweenVectors(PalmNormal, DeviceOrigin.GetLocation() - HandPos);

	// get confidence based on a cos where it should be 1 if the angle is 0 or 180 degrees,
	// and it should be 0 if it is 90 degrees
	float Confidence = (FMath::Cos(FMath::DegreesToRadians( 2 * PalmAngle)) + 1.0f) / 2.0;

	return Confidence;
}

/// <summary>
/// uses the hand velocity to calculate a confidence.
/// returns a high confidence, if the velocity is low, and a low confidence otherwise.
/// Returns 0, if the hand hasn't been consistently tracked for about the last 10 frames
/// </summary>
float FUltraleapCombinedDeviceConfidence::ConfidenceRelativeHandVelocity(
	IHandTrackingDevice* Provider, const FTransform& DeviceOrigin, const FVector HandPos, const bool IsLeft)
{
	FVector OldPosition;
	float OldTime;

	bool PositionsRecorded = IsLeft ? LastLeftHandPositions[Provider].GetOldestPosition(OldPosition, OldTime)
									: LastRightHandPositions[Provider].GetOldestPosition(OldPosition, OldTime);

	// if we haven't recorded any positions yet, or the hand hasn't been present in the last 10 frames (oldest position is older
	// than 10 * frame time), return 0
	if (!PositionsRecorded || (GetTime() - OldTime) > DeltaTimeFromTick * 10)
	{
		return 0;
	}

	float Velocity = FVector::Distance(HandPos, OldPosition) / (GetTime() - OldTime);

	float Confidence = 0;
	if (Velocity < 2)
	{
		Confidence = -0.5f * Velocity + 1;
	}

	return Confidence;
}

float FUltraleapCombinedDeviceConfidence::ConfidenceTimeSinceHandFirstVisible(IHandTrackingDevice* Provider, const bool IsLeft)
{
	if ((IsLeft ? LeftHandFirstVisible[Provider] : RightHandFirstVisible[Provider]) == 0)
	{
		return 0;
	}

	float LengthVisible = GetTime() - (IsLeft ? LeftHandFirstVisible[Provider] : RightHandFirstVisible[Provider]);

	float Confidence = 1;
	if (LengthVisible < 1)
	{
		Confidence = LengthVisible;
	}

	return Confidence;
}
/// <summary>
/// Combine different confidence functions to get an overall confidence for each joint in the given hand
/// uses frame_idx to find the corresponding provider that saw this hand
/// </summary>
void FUltraleapCombinedDeviceConfidence::CalculateJointConfidence(
	const int FrameIdx, const FLeapHandData& Hand, TArray<float>& RetConfidences)
{
	// get index in confidence arrays
	int idx = FrameIdx * 2 + (Hand.HandType == EHandType::LEAP_HAND_LEFT ? 0 : 1);
	const int NumProviders = DevicesToCombine.Num();
	// todo: why are these recreated as they're set above
	if (JointConfidences.Num() != NumProviders)
	{
		JointConfidences.AddZeroed(NUM_JOINT_POSITIONS);
		ConfidencesJointRot.AddZeroed(NUM_JOINT_POSITIONS);
		ConfidencesJointPalmRot.AddZeroed(NUM_JOINT_POSITIONS);
		ConfidencesJointOcclusion.AddZeroed(NUM_JOINT_POSITIONS);
	}
	FTransform DeviceOrigin = GetDeviceOrigin(DevicesToCombine[FrameIdx]->GetDevice());

	if (JointRotFactor != 0)
	{
		ConfidencesJointRot[idx] = ConfidenceRelativeJointRot(ConfidencesJointRot[idx], DeviceOrigin, Hand);
	}
	if (JointRotToPalmFactor != 0)
	{
		ConfidencesJointPalmRot[idx] = ConfidenceRelativeJointRotToPalmRot(ConfidencesJointPalmRot[idx], DeviceOrigin, Hand);
	}
	if (JointOcclusionFactor != 0)
	{
		// todo joint occlusion component
	//	ConfidencesJointOcclusion[idx] =
	//		JointOcclusions[FrameIdx].ConfidenceJointOcclusion(ConfidencesJointOcclusion[idx], DeviceOrigin, Hand);
	}

	for (int FingerIdx = 0; FingerIdx < 5; FingerIdx++)
	{
		for (int BoneIdx = 0; BoneIdx < 5; BoneIdx++)
		{
			int key = FingerIdx * 5 + FingerIdx;
			JointConfidences[idx][key] = JointRotFactor * ConfidencesJointRot[idx][key] +
										 JointRotToPalmFactor * ConfidencesJointPalmRot[idx][key] +
										 JointOcclusionFactor * ConfidencesJointOcclusion[idx][key];

			if (BoneIdx != 0)
			{
				// average with the confidence from the last joint on the same finger,
				// so that outer joints jump around less.
				// eg. when a confidence is low on the knuckle of a finger, the finger tip confidence for the same finger
				// should take that into account and be slightly lower too
				JointConfidences[idx][key] += JointConfidences[idx][key - 1];
				JointConfidences[idx][key] /= 2;
			}
		}
	}

	// average out new joint confidence with that of the last few frames
	if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
	{
		if (!JointConfidenceHistoriesLeft.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
		{
			JointConfidenceHistoriesLeft.Add(DevicesToCombine[FrameIdx]->GetDevice(), FJointConfidenceHistory());
		}
		JointConfidenceHistoriesLeft[DevicesToCombine[FrameIdx]->GetDevice()].AddConfidences(JointConfidences[idx]);
		JointConfidences[idx] = JointConfidenceHistoriesLeft[DevicesToCombine[FrameIdx]->GetDevice()].GetAveragedConfidences();
	}
	else
	{
		if (!JointConfidenceHistoriesRight.Contains(DevicesToCombine[FrameIdx]->GetDevice()))
		{
			JointConfidenceHistoriesRight.Add(DevicesToCombine[FrameIdx]->GetDevice(), FJointConfidenceHistory());
		}
		JointConfidenceHistoriesRight[DevicesToCombine[FrameIdx]->GetDevice()].AddConfidences(JointConfidences[idx]);
		JointConfidences[idx] = JointConfidenceHistoriesRight[DevicesToCombine[FrameIdx]->GetDevice()].GetAveragedConfidences();
	}

	RetConfidences = JointConfidences[idx];
}
/// <summary>
/// Merge hands based on hand confidences and joint confidences
/// </summary>
void FUltraleapCombinedDeviceConfidence::MergeHands(TArray<const FLeapHandData*> Hands,const TArray<float>& HandConfidences,const TArray<TArray<float>>& JointConfidencesIn, FLeapHandData& HandRet)
{
	bool IsLeft = Hands[0].HandType == EHandType::LEAP_HAND_LEFT;
	FVector MergedPalmPos = Hands[0].Palm.Position * HandConfidences[0];
	Quaternion MergedPalmRot = Hands[0].Rotation.ToQuaternion();

	for (int HandsIdx = 1; HandsIdx < Hands.Num(); HandsIdx++)
	{
		// position
		MergedPalmPos += Hands[HandsIdx].Palm.Position * HandConfidences[HandsIdx];

		// rotation
		float LerpValue = HandConfidences.Take(HandsIdx).Sum() / HandConfidences.Take(HandsIdx + 1).Sum();
		MergedPalmRot = Quaternion.Lerp(Hands[HandsIdx].Rotation.ToQuaternion(), MergedPalmRot, LerpValue);
	}

	// joints
	int Count = MergedJointPositions.Num();
	MergedJointPositions.Empty();
	MergedJointPositions.AddZeroed(Count);
	TArray<VectorHand> VectorHands;
	for(auto Hand : Hands)
	{
		VectorHands.Add(VectorHand(Hand));
	}

	for (int HandsIdx = 0; HandsIdx < Hands.Num(); HandsIdx++)
	{
		for (int JointIdx = 0; JointIdx < NUM_JOINT_POSITIONS; JointIdx++)
		{
			MergedJointPositions[JointIdx] += VectorHands[HandsIdx].JointPositions[JointIdx] * JointConfidences[HandsIdx][JointIdx];
		}
	}

	// combine everything to a hand
	FLeapHandData MergedHand = new Hand();
	new VectorHand(isLeft, mergedPalmPos, mergedPalmRot, mergedJointPositions).Decode(mergedHand);

	// visualize the joint merge:
	if (debugJointOrigins && isLeft && debugHandLeft != null)
		VisualizeMergedJoints(debugHandLeft, jointConfidences);
	else if (debugJointOrigins && !isLeft && debugHandRight != null)
		VisualizeMergedJoints(debugHandRight, jointConfidences);

	return mergedHand;
}


/// <summary>
/// uses the normal vector of a joint / bone (outwards pointing one) and the direction from joint to device
/// to calculate per-joint confidence values
/// </summary>
TArray<float> FUltraleapCombinedDeviceConfidence::ConfidenceRelativeJointRot(
	const TArray<float>& Confidences, const FTransform& DeviceOrigin, const FLeapHandData& Hand)
{
	if (Confidences.Num() == 0)
	{
		Confidences. = new float[VectorHand.NUM_JOINT_POSITIONS];
	}

	foreach(var finger in hand.Fingers)
	{
		for (int bone_idx = 0; bone_idx < 4; bone_idx++)
		{
			int key = (int) finger.Type * 4 + bone_idx;

			Vector3 jointPos = finger.Bone((Bone.BoneType) bone_idx).NextJoint.ToVector3();
			Vector3 jointNormalVector = new Vector3();
			if ((int) finger.Type == 0)
				jointNormalVector = finger.Bone((Bone.BoneType) bone_idx).Rotation.ToQuaternion() * Vector3.right;
			else
				jointNormalVector = finger.Bone((Bone.BoneType) bone_idx).Rotation.ToQuaternion() * Vector3.up * -1;

			float angle = Vector3.Angle(jointPos - deviceOrigin.position, jointNormalVector);

			// get confidence based on a cos where it should be 1 if the angle is 0 or 180 degrees,
			// and it should be 0 if it is 90 degrees
			confidences[key] = (Mathf.Cos(Mathf.Deg2Rad * 2 * angle) + 1f) / 2;
		}
	}

	return confidences;
}

/// <summary>
/// uses the normal vector of a joint / bone (outwards pointing one) and the palm normal vector
/// to calculate per-joint confidence values
/// </summary>
TArray<float> FUltraleapCombinedDeviceConfidence::ConfidenceRelativeJointRotToPalmRot(
	const TArray<float>& Confidences, const FTransform& DeviceOrigin,const FLeapHandData& Hand)
{
	if (confidences == null)
	{
		confidences = new float[VectorHand.NUM_JOINT_POSITIONS];
	}

	foreach(var finger in hand.Fingers)
	{
		for (int bone_idx = 0; bone_idx < 4; bone_idx++)
		{
			int key = (int) finger.Type * 4 + bone_idx;

			Vector3 jointPos = finger.Bone((Bone.BoneType) bone_idx).NextJoint.ToVector3();
			Vector3 jointNormalVector = new Vector3();
			jointNormalVector = finger.Bone((Bone.BoneType) bone_idx).Rotation.ToQuaternion() * Vector3.up * -1;

			float angle = Vector3.Angle(hand.PalmNormal.ToVector3(), jointNormalVector);

			// get confidence based on a cos where it should be 1 if the angle is 0,
			// and it should be 0 if the angle is 180 degrees
			confidences[key] = (Mathf.Cos(Mathf.Deg2Rad * angle) + 1f) / 2;
		}
	}

	return confidences;
}
