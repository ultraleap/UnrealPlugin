/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "FUltraleapCombinedDevice.h"
#include "JointOcclusionActor.h"

class FHandPositionHistory
{
public:
	FHandPositionHistory()
	{
		Positions.AddZeroed(NumItems);
		Times.AddZeroed(NumItems);
		Index = 0;
	}

	void ClearAllPositions()
	{
		Positions.Empty();
		Positions.AddZeroed(NumItems);
	}

	void AddPosition(const FVector& Position, const float Time)
	{
		Positions[Index] = Position;
		Times[Index] = Time;
		Index = (Index + 1) % NumItems;
	}

	bool GetPastPosition(const int PastIndex, FVector& Position, float& Time)
	{
		Position = Positions[(Index - 1 - PastIndex + NumItems) % NumItems];
		Time = Times[(Index - 1 - PastIndex + NumItems) % NumItems];

		if (Position == FVector::ZeroVector)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool GetOldestPosition(FVector& Position, float& Time)
	{
		for (int i = (NumItems - 1); i >= 0; i--)
		{
			if (GetPastPosition(i, Position, Time))
			{
				return true;
			}
		}

		GetPastPosition(0, Position, Time);
		return false;
	}

protected:
	IHandTrackingDevice* Provider;
	TArray<FVector> Positions;
	TArray<float> Times;
	int Index;

	static const int NumItems = 10;
};

// small helper class to save previous joint confidences and average over them
class FJointConfidenceHistory
{

public:
	FJointConfidenceHistory(const int NumJointPositions, const int LengthIn = 60)
	{
		Length = LengthIn;
		JointConfidences.AddZeroed(Length);

		for (int i = 0; i < Length; ++i)
		{
			JointConfidences[i].AddZeroed(NumJointPositions);
		}
		Index = 0;
	}

	void ClearAll()
	{
		ValidIndices.Empty();
	}

	void AddConfidences(const TArray<float>& Confidences)
	{
		for (int JointIndex = 0; JointIndex < Confidences.Num(); JointIndex++)
		{
			JointConfidences[Index][JointIndex] = Confidences[JointIndex];
		}
		if (ValidIndices.Find(Index) == -1)
		{
			ValidIndices.Add(Index);
		}
		Index = (Index + 1) % Length;
	}

	TArray<float> GetAveragedConfidences()
	{
		TArray<float> EmptyRet;

		if (ValidIndices.Num() == 0)
		{
			return EmptyRet;
		}

		AverageConfidences.Empty();
		AverageConfidences.AddZeroed(JointConfidences[0].Num());

		for (int i = 0; i < AverageConfidences.Num(); i++)
		{
			for(int j : ValidIndices)
			{
				AverageConfidences[i] += JointConfidences[j][i] / ValidIndices.Num();
			}
		}

		return AverageConfidences;
	}

protected:
	int Length;
	TArray<TArray<float>> JointConfidences;
	TArray<float> AverageConfidences;
	int Index;
	TArray<int> ValidIndices;
};

// small helper class to save previous whole-hand confidences and average over them
class FHandConfidenceHistory
{
public:
	FHandConfidenceHistory(int LengthIn = 60)
	{
		Length = LengthIn;
		HandConfidences.AddZeroed(Length);
		Index = 0;
	}

	void ClearAll()
	{
		ValidIndices.Empty();
	}

	void AddConfidence(const float Confidence)
	{
		HandConfidences[Index] = Confidence;

		if (ValidIndices.Find(Index) == -1)
		{
			ValidIndices.Add(Index);
		}
		Index = (Index + 1) % Length;
	}

	float GetAveragedConfidence()
	{
		if (ValidIndices.Num() == 0)
		{
			return 0;
		}

		float ConfidenceSum = 0;
		for (int j : ValidIndices)
		{
			ConfidenceSum += HandConfidences[j];
		}

		return ConfidenceSum / ValidIndices.Num();
	}

protected:
	int Length;
	TArray<float> HandConfidences;
	int Index;
	TArray<int> ValidIndices;
};
 

class FUltraleapCombinedDeviceConfidence : public FUltraleapCombinedDevice
{
public:
	FUltraleapCombinedDeviceConfidence(IHandTrackingWrapper* LeapDeviceWrapperIn, ITrackingDeviceWrapper* TrackingDeviceWrapperIn,
		TArray<IHandTrackingWrapper*> DevicesToCombineIn);
	
	virtual void UpdateJointOcclusions(AJointOcclusionActor* Actor) override;
	virtual bool GetJointOcclusionConfidences(const FString& DeviceSerial, TArray<float>& Left, TArray<float>& Right) override;
	virtual void GetDebugInfo(int32& NumCombinedLeft, int32& NumCombinedRight) override;

protected:
	virtual void CombineFrame(const TArray<FLeapFrameData>& SourceFrames) override;
	

public:
	 //If true, the overall hand confidence is affected by the duration a new hand has been visible for. When a new hand is seen for the first time, its confidence is 0. After a hand has been visible for a second, its confidence is determined by the below palm factors and palm confidences
     bool IgnoreRecentNewHands = true;

     // factors that get multiplied to the corresponding confidence values to get an overall weighted confidence value
     //How much should the Palm position relative to the tracking camera influence the overall hand confidence? A confidence value is determined by whether the hand is within the optimal FOV of the tracking camera")]
     //[Range(0f, 1f)]
     float PalmPosFactor = 1;
     // How much should the Palm orientation relative to the tracking camera influence the overall hand confidence? A confidence value is determined by looking at the angle between the palm normal and the direction from hand to camera
     // [Range(0f, 1f)]
     float PalmRotFactor = 0.2;
       
    //How much should the Palm velocity relative to the tracking camera influence the overall hand confidence?")]
    //  [Range(0f, 1f)]
    float PalmVelocityFactor = 0;

    //How much should the joint rotation relative to the tracking camera influence the overall hand confidence? A confidence value is determined for a joint by looking at the angle between the joint normal and the direction from hand to camera.
    //  [Range(0f, 1f)]
    float JointRotFactor = 1;
      
    //How much should the joint rotation relative to the palm normal influence the overall hand confidence?
    //  [Range(0f, 1f)]
    float JointRotToPalmFactor = 0.2;
    //How much should joint occlusion influence the overall hand confidence?
    //   [Range(0f, 1f)]
    float JointOcclusionFactor = 0;


    bool DebugJointOrigins = false;
     
    TMap<IHandTrackingDevice*, FHandPositionHistory> LastLeftHandPositions;
	TMap<IHandTrackingDevice*, FHandPositionHistory> LastRightHandPositions;

	TMap<IHandTrackingDevice*, float> LeftHandFirstVisible;
	TMap<IHandTrackingDevice*, float> RightHandFirstVisible;
private:

	TArray<TArray<float>> JointConfidences;
	TArray<TArray<float>> ConfidencesJointRot;
	TArray<TArray<float>> ConfidencesJointPalmRot;
	TArray<TArray<float>> ConfidencesJointOcclusion;

    TMap<IHandTrackingDevice*, FJointConfidenceHistory> JointConfidenceHistoriesLeft;
	TMap<IHandTrackingDevice*, FJointConfidenceHistory> JointConfidenceHistoriesRight;
	

    TMap<IHandTrackingDevice*, FHandConfidenceHistory> HandConfidenceHistoriesLeft;
	TMap<IHandTrackingDevice*, FHandConfidenceHistory> HandConfidenceHistoriesRight;

	int32 NumLeftHands = 0;
	int32 NumRightHands = 0;

	void MergeFrames(const TArray<FLeapFrameData>& SourceFrames, FLeapFrameData& CombinedFrame);
	void AddFrameToTimeVisibleDicts(const TArray<FLeapFrameData>& Frames, const int FrameIdx);
	float CalculateHandConfidence(int FrameIdx, const FLeapHandData& Hand);
	float ConfidenceRelativeHandPos(IHandTrackingDevice* Provider, const FTransform& DeviceOrigin, const FVector& HandPos);
	float ConfidenceRelativeHandRot(const FTransform& DeviceOrigin, const FVector& HandPos, const FVector& PalmNormal);
	float ConfidenceRelativeHandVelocity(
		IHandTrackingDevice* Provider, const FTransform& DeviceOrigin, const FVector HandPos, const bool isLeft);
	float ConfidenceTimeSinceHandFirstVisible(IHandTrackingDevice* Provider, const bool isLeft);

	void CalculateJointConfidence(
		const int FrameIdx, const FLeapHandData& Hand, TArray<float>& RetConfidences);

	void ConfidenceRelativeJointRot(TArray<float>& Confidences, const FTransform& DeviceOrigin, const FLeapHandData& Hand);
	void ConfidenceRelativeJointRotToPalmRot(
		TArray<float>& Confidences, const FTransform& DeviceOrigin, const FLeapHandData& Hand);

	void StoreConfidenceJointOcclusion(AJointOcclusionActor*, TArray<float>& Confidences, const FTransform& DeviceOrigin,
		const EHandType HandType, IHandTrackingWrapper* Provider);


	void MergeHands(const TArray<const FLeapHandData*>& Hands, const TArray<float>& HandConfidences,
		const TArray<TArray<float>>& JointConfidences, FLeapHandData& HandRet);
};
