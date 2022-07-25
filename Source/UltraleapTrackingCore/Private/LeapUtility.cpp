/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapUtility.h"

#include "Engine/Engine.h"	  // for GEngine
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "IXRTrackingSystem.h"

DEFINE_LOG_CATEGORY(UltraleapTrackingLog);

// Static vars
#define LEAP_TO_UE_SCALE 0.1f
#define UE_TO_LEAP_SCALE 10.f

FQuat FLeapUtility::LeapRotationOffset;

// Todo: use and verify this for all values
float LeapGetWorldScaleFactor()
{
	if (GEngine != nullptr && GEngine->GetWorld() != nullptr)
	{
		return (GEngine->GetWorld()->GetWorldSettings()->WorldToMeters) / 100.f;
	}
	return 1.f;
}
void FLeapUtility::LogRotation(const FString& Text, const FRotator& Rotation)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s %f %f %f"), *Text, Rotation.Yaw, Rotation.Pitch, Rotation.Roll));
	}
}
FRotator FLeapUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat * AQuat);
}

// Single point to handle leap conversion
FVector FLeapUtility::ConvertLeapVectorToFVector(const LEAP_VECTOR& LeapVector)
{
	// Expects VR Orientation
	return FVector(LeapVector.y, -LeapVector.x, -LeapVector.z);
}

FQuat FLeapUtility::ConvertLeapQuatToFQuat(const LEAP_QUATERNION& Quaternion)
{
	FQuat Quat;

	// it's -Z, X, Y tilted back by 90 degree which is -y,x,z
	Quat.X = -Quaternion.y;
	Quat.Y = Quaternion.x;
	Quat.Z = Quaternion.z;
	Quat.W = Quaternion.w;

	if (Quat.ContainsNaN())
	{
		Quat = FQuat::MakeFromEuler(FVector(0, 0, 0));
		UE_LOG(
			UltraleapTrackingLog, Log, TEXT("FLeapUtility::ConvertLeapQuatToFQuat() Warning - NAN received from tracking device"));
	}
	return Quat * LeapRotationOffset;
}


FVector FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
	const LEAP_VECTOR& LeapVector, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	// Scale from mm to cm (ue default)
	FVector ConvertedVector =
		(ConvertLeapVectorToFVector(LeapVector) + LeapMountTranslationOffset) * (LEAP_TO_UE_SCALE * LeapGetWorldScaleFactor());
	if (ConvertedVector.ContainsNaN())
	{
		ConvertedVector = FVector::ZeroVector;
		UE_LOG(UltraleapTrackingLog, Log,
			TEXT("FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets Warning - NAN received from tracking device"));
	}
	// Rotate our vector to adjust for any global rotation offsets
	return LeapMountRotationOffset.RotateVector(ConvertedVector);
}

FQuat FLeapUtility::ConvertToFQuatWithHMDOffsets(LEAP_QUATERNION Quaternion, const FQuat& LeapMountRotationOffset)
{
	FQuat UEQuat = ConvertLeapQuatToFQuat(Quaternion);
	return LeapMountRotationOffset * UEQuat;
}


FMatrix FLeapUtility::SwapLeftHandRuleForRight(const FMatrix& UEMatrix)
{
	FMatrix Matrix = UEMatrix;
	// Convenience method to swap the axis correctly, already in UE format to swap Y instead of leap Z
	FVector InverseVector = -Matrix.GetUnitAxis(EAxis::Y);
	Matrix.SetAxes(NULL, &InverseVector, NULL, NULL);
	return Matrix;
}

LEAP_VECTOR FLeapUtility::ConvertUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y;
	vector.y = UEVector.Z;
	vector.z = -UEVector.X;
	return vector;
}

LEAP_VECTOR FLeapUtility::ConvertAndScaleUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y * UE_TO_LEAP_SCALE;
	vector.y = UEVector.Z * UE_TO_LEAP_SCALE;
	vector.z = -UEVector.X * UE_TO_LEAP_SCALE;
	return vector;
}

float FLeapUtility::ScaleLeapFloatToUE(float LeapFloat)
{
	return LeapFloat * LEAP_TO_UE_SCALE;	// mm->cm
}

float FLeapUtility::ScaleUEToLeap(float UEFloat)
{
	return UEFloat * UE_TO_LEAP_SCALE;	  // mm->cm
}
// static, this has to be done during runtime as the static initialiser
// does not work in shipping builds.
void FLeapUtility::InitLeapStatics()
{
	LeapRotationOffset = FQuat(FRotator(90.f, 0.f, 180.f));
}