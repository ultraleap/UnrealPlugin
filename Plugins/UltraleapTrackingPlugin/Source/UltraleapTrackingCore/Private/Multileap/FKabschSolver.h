/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "CoreMinimal.h"

class FromMatrixExtension
{
public:
	static FVector GetVector3(FMatrix M)
	{
		return M.GetColumn(3);
	}
	static FQuat GetQuaternion(FMatrix M)
	{
		if (M.GetColumn(2) == M.GetColumn(1))
		{
			return FQuat::Identity;
		}
		// double check, was look rotation in Unity
		return FQuat::FindBetween(M.GetColumn(2), M.GetColumn(1));
	}
};

class FKabschSolver
{
public:
	FKabschSolver();
	
	FMatrix SolveKabsch(const TArray<FVector>& InPoints, const TArray<FVector>& RefPoints,
		const int OptimalRotationIterations = 9,
		const bool SolveScale = false);

	FVector& GetTranslation()
	{
		return Translation;
	}

protected:
		// https://animation.rwth-aachen.de/media/papers/2016-MIG-StableRotation.pdf
		void ExtractRotation(const TArray<FVector>& A, FQuat& Q, const int OptimalRotationIterations = 9);

private:
	// Calculate Covariance Matrices --------------------------------------------------
	void TransposeMult(const TArray<FVector>& Vec1, const TArray<FVector>& Vec2, TArray<FVector>& Covariance);
	

	
	TArray<FVector> DataCovariance;
	FVector Translation = FVector::ZeroVector;
	FQuat OptimalRotation = FQuat::Identity;
	float ScaleRatio = 1.0f;
};
