// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define MAX_HMD_SNAPSHOT_COUNT 30

/**
* Structure holding a Head Mounted Display orientation and position at a given timestamp.
*/
struct BODYSTATE_API BodyStateHMDSnapshot
{
public:
	double Timestamp;
	FVector Position;
	FQuat Orientation;

	BodyStateHMDSnapshot() {};
	BodyStateHMDSnapshot(double InTimeStamp, const FVector& InPosition, const FQuat& InOrientation);

	/** Returns the difference between the two snapshots*/
	BodyStateHMDSnapshot Difference(BodyStateHMDSnapshot& Other);

	/** For the time warp adjustment itself*/
	FTransform Transform();

	//Lerp
	BodyStateHMDSnapshot InterpolateWithOtherAtTimeStamp(BodyStateHMDSnapshot& Other, double DesiredTimeStamp);

	//Operator overloads
	BodyStateHMDSnapshot operator*(float Mult);
	void operator*=(float Mult);
};


/**
* Keep last MAX_HMD_SNAPSHOT_COUNT samples for finding the closest sample to a specified timestamp
*/
class BODYSTATE_API BSHMDSnapshotHandler
{
public:
	//Time warp utility methods
	void AddCurrentHMDSample(double CustomTimeStamp = -1);
	static BodyStateHMDSnapshot CurrentHMDSample(double CustomTimeStamp = -1);
	BodyStateHMDSnapshot LastHMDSample();
	BodyStateHMDSnapshot HMDSampleClosestToTimestamp(double Timestamp);

private:
	BodyStateHMDSnapshot Samples[MAX_HMD_SNAPSHOT_COUNT];
	int CurrentIndex = 0;
};
