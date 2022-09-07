/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "LeapC.h"

DECLARE_LOG_CATEGORY_EXTERN(UltraleapTrackingLog, Log, All);

class FLeapUtility
{
public:
	// Rotation
	static void LogRotation(const FString& Text, const FRotator& Rotation);
	static FRotator CombineRotators(FRotator A, FRotator B);

	
	// Conversion
	// To ue
	static FVector ConvertLeapVectorToFVector(const LEAP_VECTOR& LeapVector);
	static FQuat ConvertLeapQuatToFQuat(const LEAP_QUATERNION& Quaternion);

	static FVector ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		const LEAP_VECTOR& LeapVector, const FVector& LeapMountTranslationOffset,const FQuat& LeapMountRotationOffset);
	static FQuat ConvertToFQuatWithHMDOffsets(LEAP_QUATERNION Quaternion, const FQuat& LeapMountRotationOffset);

	
	static FMatrix SwapLeftHandRuleForRight(
		const FMatrix& UEMatrix);	 // needed for all left hand basis which will be incorrect in ue format

	// To leap
	static LEAP_VECTOR ConvertUEToLeap(FVector UEVector);
	static LEAP_VECTOR ConvertAndScaleUEToLeap(FVector UEVector);

	static float ScaleLeapFloatToUE(float LeapFloat);
	static float ScaleUEToLeap(float UEFloat);

	static void InitLeapStatics();
	static FQuat LeapRotationOffset;
};

class LeapUtilityTimer
{
	int64 TickTime = 0;
	int64 TockTime = 0;

public:
	LeapUtilityTimer()
	{
		tick();
	}

	double unixTimeNow()
	{
		FDateTime timeUtc = FDateTime::UtcNow();
		return timeUtc.ToUnixTimestamp() * 1000 + timeUtc.GetMillisecond();
	}

	void tick()
	{
		TickTime = unixTimeNow();
	}

	int32 tock()
	{
		TockTime = unixTimeNow();
		return TockTime - TickTime;
	}
};