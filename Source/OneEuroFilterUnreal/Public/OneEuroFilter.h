// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

struct LowPassFilter
{
	double Y, A, S;
	bool bInitialized;

public:
	void SetAlpha(double InAlpha)
	{
		if (InAlpha <= 0.0f || InAlpha > 1.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("LowPassFilter: alpha should be in range (0.0., 1.0]. Setting to 1."));
			A = 1.0;
			return;
		}
		A = InAlpha;
	}

	LowPassFilter()
	{
		bInitialized = false;
	}

	LowPassFilter(double InAlpha, double InInitVal = 0.0f)
	{
		Y = S = InInitVal;
		SetAlpha(InAlpha);
		bInitialized = false;
	}

	double Filter(double InValue)
	{
		double Result;
		if (bInitialized)
			Result = A * InValue + (1.0f - A) * S;
		else
		{
			Result = InValue;
			bInitialized = true;
		}
		Y = InValue;
		S = Result;
		return Result;
	}

	double FilterWithAlpha(double InValue, double InAlpha)
	{
		SetAlpha(InAlpha);
		return Filter(InValue);
	}

	bool HasLastRawValue()
	{
		return bInitialized;
	}

	double LastRawValue()
	{
		return Y;
	}
};

template <class FDataType>
class OneEuroFilter
{
private:
	double Freq;
	double MinCutoff;
	double Beta;
	double DCutoff;

	LowPassFilter* X;
	LowPassFilter* DX;
	int FiltersCount;

	double LastTime;

	// CurrValue contains the latest value which have been succesfully filtered
	// PrevValue contains the previous filtered value
	FDataType CurrValue;
	FDataType PrevValue;

public:
	OneEuroFilter();

	OneEuroFilter(double InFrequency, double InMinCutoff = 1.0f, double InBeta = 0.0f, double InDCutoff = 1.0f);

	~OneEuroFilter();

	void UpdateParams(double InFrequency, double InMinCutoff, double InBeta, double InDCutoff);

	FDataType Filter(FDataType InValue, double InTimeStamp = -1.0f);

	FDataType GetCurrValue();

	void Reset();

	double Alpha(double InCutoff);

	void SetFrequency(double InFrequency);

	void SetMinCutoff(double InMinCutoff);

	void SetBeta(double InBeta);

	void SetDerivateCutoff(double InDerivativeCutoff);

private:
	inline void InitFilters();

	inline double EstimateAndFilter(double InValue, int InFilterIndex = 0);

	inline FDataType FilterInValue(FDataType InValue);
};

template <class FDataType>
OneEuroFilter<FDataType>::OneEuroFilter()
{
}

template <class FDataType>
OneEuroFilter<FDataType>::OneEuroFilter(double InFrequency, double InMinCutoff, double InBeta, double InDCutoff)
{
	SetFrequency(InFrequency);
	SetMinCutoff(InMinCutoff);
	SetBeta(InBeta);
	SetDerivateCutoff(InDCutoff);
	LastTime = -1.0;

	InitFilters();	  // based on template type
}

template <class FDataType>
OneEuroFilter<FDataType>::~OneEuroFilter()
{
	delete[] X;
	delete[] DX;
	FiltersCount = 0;
}

template <class FDataType>
void OneEuroFilter<FDataType>::UpdateParams(double InFrequency, double InMinCutoff, double InBeta, double InDCutoff)
{
	SetFrequency(InFrequency);
	SetMinCutoff(InMinCutoff);
	SetBeta(InBeta);
	SetDerivateCutoff(InDCutoff);

	// init all filters (based on template type, they might be more than one
	if (FiltersCount > 0)
		for (int i = 0; i < FiltersCount; i++)
		{
			X[i].SetAlpha(Alpha(MinCutoff));
			DX[i].SetAlpha(Alpha(DCutoff));
		}
}

template <class FDataType>
FDataType OneEuroFilter<FDataType>::Filter(FDataType InValue, double InTimeStamp)
{
	PrevValue = CurrValue;

	// update the sampling frequency based on timestamps
	if (LastTime != -1.0f && InTimeStamp != -1.0f)
		Freq = 1.0f / (InTimeStamp - LastTime);

	LastTime = InTimeStamp;

	CurrValue = FilterInValue(InValue);

	return CurrValue;
}

template <class FDataType>
FDataType OneEuroFilter<FDataType>::FilterInValue(FDataType InValue)
{
	return EstimateAndFilter(InValue);
}

template <class FDataType>
double OneEuroFilter<FDataType>::EstimateAndFilter(double InValue, int InFilterIndex)
{
	if (InFilterIndex < FiltersCount)
	{
		// estimate the current variation per second
		double DValue = X[InFilterIndex].HasLastRawValue() ? (InValue - X[InFilterIndex].LastRawValue()) * Freq
														   : 0.0f;	  // FIXME: 0.0 or InValue?
		double EDValue = DX[InFilterIndex].FilterWithAlpha(DValue, Alpha(DCutoff));

		// use it to update the Cutoff frequency
		double Cutoff = MinCutoff + Beta * FMath::Abs(EDValue);

		// filter the given InValue
		return X[InFilterIndex].FilterWithAlpha(InValue, Alpha(Cutoff));
	}
	else
		return InValue;
}

template <class FDataType>
FDataType OneEuroFilter<FDataType>::GetCurrValue()
{
	return CurrValue;
}

template <class FDataType>
void OneEuroFilter<FDataType>::Reset()
{
	InitFilters();
}

template <class FDataType>
double OneEuroFilter<FDataType>::Alpha(double InCutoff)
{
	double Te = 1.0f / Freq;
	double Tau = 1.0f / (2.0f * PI * InCutoff);
	return 1.0f / (1.0f + Tau / Te);
}

template <class FDataType>
void OneEuroFilter<FDataType>::SetFrequency(double InFrequency)
{
	if (InFrequency <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Freq should be > 0. Setting to 120."));
		Freq = 120.0;
		return;
	}
	Freq = InFrequency;
}

template <class FDataType>
void OneEuroFilter<FDataType>::SetMinCutoff(double InMinCutoff)
{
	if (InMinCutoff <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("MinCutoff should be > 0. Setting to 1."));
		MinCutoff = 1.0;
		return;
	}
	MinCutoff = InMinCutoff;
}

template <class FDataType>
void OneEuroFilter<FDataType>::SetBeta(double InBeta)
{
	Beta = InBeta;
}

template <class FDataType>
void OneEuroFilter<FDataType>::SetDerivateCutoff(double InDerivativeCutoff)
{
	if (InDerivativeCutoff <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("DCutoff should be > 0. Setting to 1."));
		DCutoff = 1.0;
		return;
	}
	DCutoff = InDerivativeCutoff;
}

template <class FDataType>
void OneEuroFilter<FDataType>::InitFilters()
{
	X = new LowPassFilter(Alpha(MinCutoff));
	DX = new LowPassFilter(Alpha(DCutoff));

	CurrValue = 0.0;
	PrevValue = CurrValue;

	FiltersCount = 1;
}

// Specialization of functions based on type

template <>
FVector OneEuroFilter<FVector>::FilterInValue(FVector InValue)
{
	FVector Result;

	for (int i = 0; i < 3; i++)
		Result[i] = EstimateAndFilter(InValue[i], i);

	return Result;
}

template <>
FRotator OneEuroFilter<FRotator>::FilterInValue(FRotator InValue)
{
	FRotator Result;

	Result.Roll = EstimateAndFilter(InValue.Roll, 0);
	Result.Pitch = EstimateAndFilter(InValue.Pitch, 1);
	Result.Yaw = EstimateAndFilter(InValue.Yaw, 2);

	return Result;
}

template <>
FQuat OneEuroFilter<FQuat>::FilterInValue(FQuat InValue)
{
	FQuat IncomingQuat = InValue;

	// fix for potential weird cases (see DM's Unity OneEuroFilterPlugin)
	FVector4 PrevQuatAsVector(PrevValue.X, PrevValue.Y, PrevValue.Z, PrevValue.W);
	FVector4 CurrQuatAsVector(IncomingQuat.X, IncomingQuat.Y, IncomingQuat.Z, IncomingQuat.W);
	if ((PrevQuatAsVector - CurrQuatAsVector).SizeSquared() > 2)
		IncomingQuat = FQuat(-IncomingQuat.X, -IncomingQuat.Y, -IncomingQuat.Z, -IncomingQuat.W);

	FQuat Result;

	Result.X = EstimateAndFilter(IncomingQuat.X, 0);
	Result.Y = EstimateAndFilter(IncomingQuat.Y, 1);
	Result.Z = EstimateAndFilter(IncomingQuat.Z, 2);
	Result.W = EstimateAndFilter(IncomingQuat.W, 3);

	return Result;
}

template <>
void OneEuroFilter<FVector>::InitFilters()
{
	X = new LowPassFilter[3];
	DX = new LowPassFilter[3];

	for (int i = 0; i < 3; i++)
	{
		X[i] = LowPassFilter(Alpha(MinCutoff));
		DX[i] = LowPassFilter(Alpha(DCutoff));
	}

	CurrValue = FVector::ZeroVector;
	PrevValue = CurrValue;

	FiltersCount = 3;
}

template <>
void OneEuroFilter<FRotator>::InitFilters()
{
	X = new LowPassFilter[4];
	DX = new LowPassFilter[4];

	for (int i = 0; i < 3; i++)
	{
		X[i] = LowPassFilter(Alpha(MinCutoff));
		DX[i] = LowPassFilter(Alpha(DCutoff));
	}

	CurrValue = FRotator::ZeroRotator;
	PrevValue = CurrValue;

	FiltersCount = 4;
}

template <>
void OneEuroFilter<FQuat>::InitFilters()
{
	X = new LowPassFilter[4];
	DX = new LowPassFilter[4];

	for (int i = 0; i < 4; i++)
	{
		X[i] = LowPassFilter(Alpha(MinCutoff));
		DX[i] = LowPassFilter(Alpha(DCutoff));
	}

	CurrValue = FQuat::Identity;
	PrevValue = CurrValue;

	FiltersCount = 4;
}
