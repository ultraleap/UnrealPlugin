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

	BodyStateHMDSnapshot()
	{
		Position = FVector::ZeroVector;
		Orientation = FQuat::Identity;
		Timestamp = 0;
	}
	BodyStateHMDSnapshot(double InTimeStamp, const FVector& InPosition, const FQuat& InOrientation);

	/** Returns the difference between the two snapshots*/
	BodyStateHMDSnapshot Difference(BodyStateHMDSnapshot& Other);

	/** For the time warp adjustment itself*/
	FTransform Transform();

	// Lerp
	BodyStateHMDSnapshot InterpolateWithOtherAtTimeStamp(BodyStateHMDSnapshot& Other, double DesiredTimeStamp);

	// Operator overloads
	BodyStateHMDSnapshot operator*(float Mult);
	void operator*=(float Mult);
};

/**
 * Keep last MAX_HMD_SNAPSHOT_COUNT samples for finding the closest sample to a specified timestamp
 */
class BODYSTATE_API BSHMDSnapshotHandler
{
public:
	// Time warp utility methods
	void AddCurrentHMDSample(double CustomTimeStamp = -1);
	static BodyStateHMDSnapshot CurrentHMDSample(double CustomTimeStamp = -1);
	BodyStateHMDSnapshot LastHMDSample();
	BodyStateHMDSnapshot HMDSampleClosestToTimestamp(double Timestamp);

private:
	BodyStateHMDSnapshot Samples[MAX_HMD_SNAPSHOT_COUNT];
	int CurrentIndex = 0;
};
