/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "FUltraleapCombinedDevice.h"


// placeholder for complile test
class JointOcclusion
{

};

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
	ITrackingDeviceWrapper* Provider;
	TArray<FVector> Positions;
	TArray<float> Times;
	int Index;

	static const int NumItems = 10;
};

// small helper class to save previous joint confidences and average over them
class FJointConfidenceHistory
{

public:
	FJointConfidenceHistory(const int LengthIn = 60)
	{
		Length = LengthIn;
		// TODO get vector hand equivalent
		//JointConfidences = new float[Length, 1 /* VectorHand.NUM_JOINT_POSITIONS*/];
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

		// TODO: check,
		// size was JointConfidences.GetLength(1) in Unity
		AverageConfidences.Empty();
		AverageConfidences.AddZeroed(Length);

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
	FHandConfidenceHistory(int Length = 60)
	{
		Length = Length;
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
		TArray<IHandTrackingWrapper*> DevicesToCombineIn)
		: FUltraleapCombinedDevice(LeapDeviceWrapperIn, TrackingDeviceWrapperIn, DevicesToCombineIn)
	{
	}
	
protected:
	virtual void CombineFrame(const TArray<FLeapFrameData>& SourceFrames) override
	{
	}

public:
	 //If true, the overall hand confidence is affected by the duration a new hand has been visible for. When a new hand is seen for the first time, its confidence is 0. After a hand has been visible for a second, its confidence is determined by the below palm factors and palm confidences
     bool ignoreRecentNewHands = true;

     // factors that get multiplied to the corresponding confidence values to get an overall weighted confidence value
     //How much should the Palm position relative to the tracking camera influence the overall hand confidence? A confidence value is determined by whether the hand is within the optimal FOV of the tracking camera")]
     //[Range(0f, 1f)]
     float palmPosFactor = 0;
     // How much should the Palm orientation relative to the tracking camera influence the overall hand confidence? A confidence value is determined by looking at the angle between the palm normal and the direction from hand to camera
     // [Range(0f, 1f)]
     float palmRotFactor = 0;
       
      //How much should the Palm velocity relative to the tracking camera influence the overall hand confidence?")]
      //  [Range(0f, 1f)]
      float palmVelocityFactor = 0;

      //How much should the joint rotation relative to the tracking camera influence the overall hand confidence? A confidence value is determined for a joint by looking at the angle between the joint normal and the direction from hand to camera.
      //  [Range(0f, 1f)]
      float jointRotFactor = 0;
      
      //How much should the joint rotation relative to the palm normal influence the overall hand confidence?
      //  [Range(0f, 1f)]
      float jointRotToPalmFactor = 0;
      //How much should joint occlusion influence the overall hand confidence?
      //   [Range(0f, 1f)]
      float jointOcclusionFactor = 0;


      bool debugJointOrigins = false;
     
    TMap<ITrackingDeviceWrapper*, FHandPositionHistory> LastLeftHandPositions;
	TMap<ITrackingDeviceWrapper*, FHandPositionHistory> LastRightHandPositions;

	TMap<ITrackingDeviceWrapper*, float> LeftHandFirstVisible;
	TMap<ITrackingDeviceWrapper*, float> RightHandFirstVisible;

    TArray<JointOcclusion> JointOcclusions;

	// todo: init size as in Unity = new FVector3[VectorHand.NUM_JOINT_POSITIONS];
    TArray<FVector> mergedJointPositions;
	

private:

	TArray<TArray<float>> JointConfidences;
	TArray<TArray<float>> Confidences_jointRot;
	TArray<TArray<float>> Confidences_jointPalmRot;
	TArray<TArray<float>> Confidences_jointOcclusion;

    TMap<ITrackingDeviceWrapper*, FJointConfidenceHistory> jointConfidenceHistoriesLeft;
	TMap<ITrackingDeviceWrapper*, FJointConfidenceHistory> jointConfidenceHistoriesRight;
	

    TMap<ITrackingDeviceWrapper*, FHandConfidenceHistory> handConfidenceHistoriesLeft;
	TMap<ITrackingDeviceWrapper*, FHandConfidenceHistory> handConfidenceHistoriesRight;

};
