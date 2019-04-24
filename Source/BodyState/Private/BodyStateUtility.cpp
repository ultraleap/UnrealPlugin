// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BodyStateUtility.h"

DEFINE_LOG_CATEGORY(BodyStateLog);

FRotator FBodyStateUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat*AQuat);
}

float FBodyStateUtility::AngleBetweenVectors(FVector A, FVector B)
{
	float dotAB = FVector::DotProduct(A, B);
	float bottom = (A.Size()*B.Size());
	if (bottom != 0)
	{
		return FMath::Acos(dotAB / bottom);
	}
	else
	{
		return 0.f;
	}
}
