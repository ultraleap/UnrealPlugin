/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FKabschSolver.h"


FKabschSolver::FKabschSolver()
{
	DataCovariance.AddZeroed(3);
	Translation = FVector::ZeroVector;
	OptimalRotation = FQuat::Identity;
}
FMatrix FKabschSolver::SolveKabsch(const TArray<FVector>& InPoints, const TArray<FVector>& RefPoints,
	const int OptimalRotationIterations , const bool SolveScale)
{
	if (InPoints.Num() != RefPoints.Num())
	{
		return FMatrix::Identity;
	}
	
	TArray<FVector> InPts(InPoints);
	TArray<FVector> RefPts(RefPoints);

	// Calculate the centroid offset and construct the centroid-shifted point matrices
	FVector InCentroid = FVector::ZeroVector;
	FVector RefCentroid = FVector::ZeroVector;

	for (int i = 0; i < InPts.Num(); i++)
	{
		InCentroid += InPts[i];
		RefCentroid += RefPts[i];
	}
	InCentroid /= InPts.Num();
	RefCentroid /= RefPts.Num();

	for (int i = 0; i < InPts.Num(); i++)
	{
		InPts[i] -= InCentroid;
		RefPts[i] -= RefCentroid;
	}

	// why is this (translation) never used - same in Unity, for debug?
	Translation = RefCentroid - InCentroid;

	// Calculate the scale ratio
	if (SolveScale && InPoints.Num() > 1)
	{
		float InScale = 0.0f, RefScale = 0.0f;

		for (int i = 0; i < InPoints.Num(); i++)
		{
			InScale += (FVector(InPoints[i].X, InPoints[i].Y, InPoints[i].Z) - InCentroid).Size();
			RefScale += (FVector(RefPoints[i].X, RefPoints[i].Y, RefPoints[i].Z) - RefCentroid).Size();
		}
		ScaleRatio = (RefScale / InScale);
	}
	else
	{
		ScaleRatio = 1.0f;
	}

	if (InPoints.Num() != 1)
	{
		// Calculate the covariance matrix, is a 3x3 Matrix and Calculate the optimal rotation
		TransposeMult(InPts, RefPts, DataCovariance);
		ExtractRotation(DataCovariance, OptimalRotation, OptimalRotationIterations);
		
		
		/* UE_LOG(UltraleapTrackingLog, Log,
			TEXT("Kabsch Rotation P %f Y %f R %f"), OptimalRotation.Rotator().Pitch, OptimalRotation.Rotator().Yaw,
				OptimalRotation.Rotator().Roll);
		*/
		 //OptimalRotation = FQuat::Identity;
	}
	else
	{
		OptimalRotation = FQuat::Identity;
	}
	FScaleRotationTranslationMatrix TSR1(FVector::OneVector * ScaleRatio, FRotator::ZeroRotator, RefCentroid);
	FScaleRotationTranslationMatrix TSR2(FVector::OneVector, OptimalRotation.Rotator(), FVector::ZeroVector);
	FScaleRotationTranslationMatrix TSR3(FVector::OneVector, FRotator::ZeroRotator, -InCentroid);
	return TSR1 * TSR2 * TSR3;
}
void FillMatrixFromQuaternion(const FQuat& Q, TArray<FVector>& Matrix)
{
	Matrix[0] = Q * FVector::ForwardVector;
	Matrix[1] = Q * FVector::RightVector;	   // In Unity Up
	Matrix[2] = Q * FVector::UpVector;			// In Unity Forward
}
void FKabschSolver::ExtractRotation(const TArray<FVector>& A, FQuat& Q, const int OptimalRotationIterations)
{
	TArray<FVector> QuatBasis;
	QuatBasis.AddZeroed(3);

	for (int Iter = 0; Iter < OptimalRotationIterations; Iter++)
	{
		FillMatrixFromQuaternion(Q, QuatBasis);

		
		FVector Omega = (	FVector::CrossProduct(QuatBasis[0], A[0]) + 
							FVector::CrossProduct(QuatBasis[1], A[1]) +
							FVector::CrossProduct(QuatBasis[2], A[2])) *
		(1.0f / FMath::Abs(	FVector::DotProduct(QuatBasis[0], A[0]) + 
							FVector::DotProduct(QuatBasis[1], A[1]) +
							FVector::DotProduct(QuatBasis[2], A[2]) + 0.000000001f));

		float W = Omega.Size();
		if (W < 0.000000001f)
		{
			break;
		}
		FVector NormalizedAngleAxisInput = Omega / W;

		NormalizedAngleAxisInput.Normalize();
		
		FQuat AngleAxis(NormalizedAngleAxisInput, W);
		Q = AngleAxis * Q;
		Q.Normalize();
	}
}

void FKabschSolver::TransposeMult(
	const TArray<FVector>& Vec1, const TArray<FVector>& Vec2, TArray<FVector>& Covariance)
{
	for (int i = 0; i < 3; i++)
	{	
		// i is the row in this matrix
		Covariance[i] = FVector::ZeroVector;
		for (int j = 0; j < 3; j++)
		{	 // j is the column in the other matrix
			for (int k = 0; k < Vec1.Num(); k++)
			{	 // k is the column in this matrix
				Covariance[i][j] += Vec1[k][i] * Vec2[k][j];
			}
		}
	}
}