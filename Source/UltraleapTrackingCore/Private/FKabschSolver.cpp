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
	QuatBasis.AddZeroed(3);
	DataCovariance.AddZeroed(3);
	Translation = FVector::ZeroVector;
}
FMatrix FKabschSolver::SolveKabsch(const TArray<FVector>& InPoints, const TArray<FVector>& RefPoints,
	const int OptimalRotationIterations , const bool SolveScale)
{
	if (InPoints.Num() != RefPoints.Num())
	{
		return FMatrix::Identity;
	}
	// TODO: why copy
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
		ExtractRotation(TransposeMult(InPts, RefPts, DataCovariance),OptimalRotation, OptimalRotationIterations);
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
void FKabschSolver::ExtractRotation(const TArray<FVector>& A, FQuat& Q, const int OptimalRotationIterations)
{
}
TArray<FVector> FKabschSolver::TransposeMult(
	const TArray<FVector>& Vec1, const TArray<FVector>& Vec2, const TArray<FVector>& Covariance)
{
	TArray<FVector> Ret;

	return Ret;
}
TArray<FVector> FKabschSolver::MatrixFromQuaternion(const FQuat& Q, const TArray<FVector>& Covariance)
{
	TArray<FVector> Ret;

	return Ret;
}