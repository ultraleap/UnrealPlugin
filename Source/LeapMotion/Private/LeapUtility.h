// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapC.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LeapMotionLog, Log, All);

class FLeapUtility 
{
public:
	//Rotation
	static FRotator CombineRotators(FRotator A, FRotator B);

	static void SetLeapGlobalOffsets(const FVector& TranslationOffset, const FRotator& RotationOffset);

	//Conversion
	//To ue
	static FVector ConvertLeapVectorToFVector(const LEAP_VECTOR& LeapVector);
	static FQuat ConvertLeapQuatToFQuat(const LEAP_QUATERNION& Quaternion);

	static FVector ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(const LEAP_VECTOR& LeapVector);
	static FQuat ConvertToFQuatWithHMDOffsets(LEAP_QUATERNION Quaternion);

	static FMatrix ConvertLeapBasisMatrix(LEAP_DISTORTION_MATRIX LeapMatrix);
	static FMatrix SwapLeftHandRuleForRight(const FMatrix &UEMatrix);		//needed for all left hand basis which will be incorrect in ue format

																	//To leap
	static LEAP_VECTOR ConvertUEToLeap(FVector UEVector);
	static LEAP_VECTOR ConvertAndScaleUEToLeap(FVector UEVector);

	static float ScaleLeapFloatToUE(float LeapFloat);
	static float ScaleUEToLeap(float UEFloat);


	static FVector LeapMountTranslationOffset;
	static FQuat LeapMountRotationOffset;
	static FQuat FacingAdjustQuat;
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