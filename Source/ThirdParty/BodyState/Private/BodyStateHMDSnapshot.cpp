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

#include "BodyStateHMDSnapshot.h"

#include "BodyStateUtility.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"

void BSHMDSnapshotHandler::AddCurrentHMDSample(double CustomTimeStamp)
{
	// Grab current sample
	Samples[CurrentIndex] = CurrentHMDSample(CustomTimeStamp);

	// Circular tracker - slot it in correctly
	CurrentIndex++;
	if (CurrentIndex >= MAX_HMD_SNAPSHOT_COUNT)
	{
		CurrentIndex = 0;
	}
}

BodyStateHMDSnapshot::BodyStateHMDSnapshot(double InTimeStamp, const FVector& InPosition, const FQuat& InOrientation)
{
	Timestamp = InTimeStamp;
	Position = InPosition;
	Orientation = InOrientation;
}
BodyStateHMDSnapshot BodyStateHMDSnapshot::Difference(BodyStateHMDSnapshot& Other)
{
	BodyStateHMDSnapshot Result;
	Result.Timestamp = Timestamp - Other.Timestamp;

	FTransform DeltaTransform = Transform().GetRelativeTransform(Other.Transform());

	Result.Orientation = DeltaTransform.GetRotation();
	Result.Position = DeltaTransform.GetTranslation();
	return Result;
}

FTransform BodyStateHMDSnapshot::Transform()
{
	return FTransform(Orientation, Position, FVector(1.f));
}

BodyStateHMDSnapshot BodyStateHMDSnapshot::InterpolateWithOtherAtTimeStamp(BodyStateHMDSnapshot& Other, double DesiredTimeStamp)
{
	// Is the timestamp between these two samples?
	if ((Timestamp <= DesiredTimeStamp && DesiredTimeStamp <= Other.Timestamp) ||
		(Other.Timestamp <= DesiredTimeStamp && DesiredTimeStamp <= Timestamp))
	{
		BodyStateHMDSnapshot result;

		double range = FMath::Abs(Other.Timestamp - Timestamp);
		float FactorThis = FMath::Abs(DesiredTimeStamp - Timestamp) / range;
		float FactorOther = FMath::Abs(DesiredTimeStamp - Other.Timestamp) / range;

		result.Position = Position * FactorThis + Other.Position * FactorOther;
		result.Orientation = FQuat::Slerp(Orientation, Other.Orientation, FactorThis);
		result.Timestamp = DesiredTimeStamp;
		return result;
	}
	else
	{
		UE_LOG(BodyStateLog, Warning, TEXT("Invalid Lerp Timestamp, this: %1.0f other: %1.0f desired: %1.0f diff: %1.0f"),
			Timestamp, Other.Timestamp, DesiredTimeStamp, Timestamp - Other.Timestamp);
		// return self
		return *this;
	}
}

BodyStateHMDSnapshot BodyStateHMDSnapshot::operator*(float Mult)
{
	// Orientation.Slerp(Orientation, )
	return BodyStateHMDSnapshot(Timestamp, Position * Mult, Orientation * Mult);
}

void BodyStateHMDSnapshot::operator*=(float Mult)
{
	this->Position *= Mult;
	this->Orientation *= Mult;
}

BodyStateHMDSnapshot BSHMDSnapshotHandler::CurrentHMDSample(double CustomTimeStamp)
{
	BodyStateHMDSnapshot Snapshot;

	if (CustomTimeStamp >= 0)
	{
		Snapshot.Timestamp = CustomTimeStamp;
	}
	else
	{
		Snapshot.Timestamp = FPlatformTime::Seconds();
	}

	if (GEngine && GEngine->XRSystem.IsValid())
	{
		GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, Snapshot.Orientation, Snapshot.Position);
	}
	return Snapshot;
}

BodyStateHMDSnapshot BSHMDSnapshotHandler::LastHMDSample()
{
	return Samples[CurrentIndex];
}

BodyStateHMDSnapshot BSHMDSnapshotHandler::HMDSampleClosestToTimestamp(double PassedTimestamp)
{
	double MinDifference = DBL_MAX;
	int32 MinIndex = 0;	   // always have a valid index in case something goes wrong

	// UE_LOG(LeapPluginLog, Log, TEXT("Time warp Debug - Now: %d"), Timestamp);

	for (int32 i = 0; i < MAX_HMD_SNAPSHOT_COUNT; i++)
	{
		BodyStateHMDSnapshot& Snapshot = Samples[i];
		int32 Difference = FMath::Abs(Snapshot.Timestamp - PassedTimestamp);

		// UE_LOG(LeapPluginLog, Log, TEXT("Time warp Debug - Snapshot: %d, Difference: %d"), Snapshot.Timestamp, Difference);

		if (Difference < MinDifference)
		{
			MinDifference = Difference;
			MinIndex = i;
		}
	}

	// Not a perfect match? lerp the sample
	if (MinDifference > 0)
	{
		BodyStateHMDSnapshot& FoundSample = Samples[MinIndex];
		int InterpIndex = MinIndex;

		// Did we find an older timer stamp? use a future timestamp to interpolate
		if (FoundSample.Timestamp < PassedTimestamp)
		{
			InterpIndex = MinIndex + 1;
		}
		else
		{
			InterpIndex = MinIndex - 1;
		}

		// Sanity check for the index
		if (InterpIndex < 0)
		{
			InterpIndex = MAX_HMD_SNAPSHOT_COUNT - 1;
		}
		if (InterpIndex == MAX_HMD_SNAPSHOT_COUNT)
		{
			InterpIndex = 0;
		}
		BodyStateHMDSnapshot InterpSample = FoundSample.InterpolateWithOtherAtTimeStamp(Samples[InterpIndex], PassedTimestamp);
		return InterpSample;
	}

	// UE_LOG(LogTemp, Log, TEXT("Time warp Debug - MinSnapshot: %d, MinDifference: %d"), Samples[MinIndex].Timestamp,
	// MinDifference);
	return Samples[MinIndex];
}
