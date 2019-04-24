// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LeapUtility.h"
#include "Engine/Engine.h" // for GEngine
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "IXRTrackingSystem.h"

DEFINE_LOG_CATEGORY(LeapMotionLog);

//Static vars
#define LEAP_TO_UE_SCALE 0.1f
#define UE_TO_LEAP_SCALE 10.f


//Defaults - NB: these don't get automatically set in a development context since 4.20
FVector FLeapUtility::LeapMountTranslationOffset = FVector(8.f, 0, 0);
FQuat FLeapUtility::LeapMountRotationOffset = FQuat(FRotator(0, 0, 0));

FQuat FLeapUtility::FacingAdjustQuat = FQuat(FRotator(90.f, 0.f, 0.f));
FQuat FLeapUtility::LeapRotationOffset = FQuat(FRotator(90.f, 0.f, 180.f));

//Todo: use and verify this for all values
float LeapGetWorldScaleFactor()
{
	if (GEngine != nullptr && GEngine->GetWorld() != nullptr)
	{
		return (GEngine->GetWorld()->GetWorldSettings()->WorldToMeters) / 100.f;
	}
	return 1.f;
}


FRotator FLeapUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat*AQuat);
}

void FLeapUtility::SetLeapGlobalOffsets(const FVector& TranslationOffset, const FRotator& RotationOffset)
{
	LeapMountTranslationOffset = TranslationOffset;
	LeapMountRotationOffset = RotationOffset.Quaternion();

	//These need to be set from a call due to static constants not being set since 4.20
	FacingAdjustQuat = FQuat(FRotator(90.f, 0.f, 0.f));
	LeapRotationOffset = FQuat(FRotator(90.f, 0.f, 180.f));
}

//Single point to handle leap conversion
FVector FLeapUtility::ConvertLeapVectorToFVector(const LEAP_VECTOR& LeapVector)
{
	//Expects VR Orientation
	return FVector(LeapVector.y, -LeapVector.x, -LeapVector.z);
}

FQuat FLeapUtility::ConvertLeapQuatToFQuat(const LEAP_QUATERNION& Quaternion)
{
	FQuat Quat;
	
	//it's -Z, X, Y tilted back by 90 degree which is -y,x,z
	Quat.X = -Quaternion.y;
	Quat.Y = Quaternion.x;
	Quat.Z = Quaternion.z;
	Quat.W = Quaternion.w;

	return Quat * FLeapUtility::LeapRotationOffset;
}

FVector AdjustForLeapFacing(FVector In)
{
	return FLeapUtility::FacingAdjustQuat.RotateVector(In);
}

FVector AdjustForHMD(FVector In)
{
	if (GEngine->XRSystem.IsValid())
	{
		FQuat OrientationQuat;
		FVector Position;
		GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, OrientationQuat, Position);
		FVector Out = OrientationQuat.RotateVector(In);
		Position += OrientationQuat.RotateVector(FLeapUtility::LeapMountTranslationOffset);
		Out += Position;
		return Out;
	}
	else
	{
		return In;
	}
}

FVector AdjustForHMDOrientation(FVector In)
{
	if (GEngine->XRSystem.IsValid())
	{
		FQuat OrientationQuat;
		FVector Position;
		GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, OrientationQuat, Position);
		FVector Out = OrientationQuat.RotateVector(In);
		return Out;
	}
	else
		return In;

}


FVector FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(const LEAP_VECTOR& LeapVector)
{
	//Scale from mm to cm (ue default)
	FVector ConvertedVector = (ConvertLeapVectorToFVector(LeapVector) + LeapMountTranslationOffset) * (LEAP_TO_UE_SCALE * LeapGetWorldScaleFactor());

	//Rotate our vector to adjust for any global rotation offsets
	return LeapMountRotationOffset.RotateVector(ConvertedVector);
}

FQuat FLeapUtility::ConvertToFQuatWithHMDOffsets(LEAP_QUATERNION Quaternion)
{
	FQuat UEQuat = ConvertLeapQuatToFQuat(Quaternion);
	return LeapMountRotationOffset * UEQuat;
}

FMatrix FLeapUtility::ConvertLeapBasisMatrix(LEAP_DISTORTION_MATRIX LeapMatrix)
{
	/*
	Leap Basis depends on hand type with -z, x, y as general format. This then needs to be inverted to point correctly (x = forward), which yields the matrix below.
	*/
	FVector InX, InY, InZ, InW;
	/*InX.Set(LeapMatrix.zBasis.z, -LeapMatrix.zBasis.x, -LeapMatrix.zBasis.y);
	InY.Set(-LeapMatrix.xBasis.z, LeapMatrix.xBasis.x, LeapMatrix.xBasis.y);
	InZ.Set(-LeapMatrix.yBasis.z, LeapMatrix.yBasis.x, LeapMatrix.yBasis.y);
	InW.Set(-LeapMatrix.origin.z, LeapMatrix.origin.x, LeapMatrix.origin.y);

	if (LeapShouldAdjustForFacing)
	{
		InX = AdjustForLeapFacing(InX);
		InY = AdjustForLeapFacing(InY);
		InZ = AdjustForLeapFacing(InZ);
		InW = AdjustForLeapFacing(InW);

		if (LeapShouldAdjustRotationForHMD)
		{
			InX = adjustForHMDOrientation(InX);
			InY = adjustForHMDOrientation(InY);
			InZ = adjustForHMDOrientation(InZ);
			InW = adjustForHMDOrientation(InW);
		}
	}
	
	Disabled for now, not sure what the equivalent is now
	*/

	return (FMatrix(InX, InY, InZ, InW));
}
FMatrix FLeapUtility::SwapLeftHandRuleForRight(const FMatrix& UEMatrix)
{
	FMatrix Matrix = UEMatrix;
	//Convenience method to swap the axis correctly, already in UE format to swap Y instead of leap Z
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
	return LeapFloat * LEAP_TO_UE_SCALE;	//mm->cm
}

float FLeapUtility::ScaleUEToLeap(float UEFloat)
{
	return UEFloat * UE_TO_LEAP_SCALE;	//mm->cm
}