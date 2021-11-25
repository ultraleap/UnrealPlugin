// Fill out your copyright notice in the Description page of Project Settings.

#include "OneEuroFilterBlueprintDatatypes.h"

#include "OneEuroFilter.h"

// FLOAT

FOneEuroFilterFloat::FOneEuroFilterFloat(float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	FOneEuroFilterSettings(InFrequency, InMinCutoff, InBeta, InDCutoff);

	Value = 0;
	FilteredValue = 0;
}

void FOneEuroFilterFloat::SetValue(float NewValue)
{
	Value = NewValue;

	if (!bInitialized)
	{
		TheFilter = MakeShareable(new OneEuroFilter<float>(Frequency, MinCutoff, Beta, DCutoff));

		bInitialized = true;
	}

	if (TheFilter.IsValid())
		FilteredValue = TheFilter->Filter(Value);
}

void FOneEuroFilterFloat::GetValue(float& Raw, float& Filtered)
{
	Raw = Value;
	Filtered = FilteredValue;
}
void FOneEuroFilterFloat::RefreshParameters()
{
	if (!TheFilter)
	{
		return;
	}
	TheFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
}
// FVECTOR
FOneEuroFilterVector::FOneEuroFilterVector(float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	FOneEuroFilterSettings(InFrequency, InMinCutoff, InBeta, InDCutoff);

	Value = FVector::ZeroVector;
	FilteredValue = FVector::ZeroVector;
	TheFilter = MakeShareable(new OneEuroFilter<FVector>(Frequency, MinCutoff, Beta, DCutoff));
}

void FOneEuroFilterVector::SetValue(FVector NewValue)
{
	if (!bInitialized)
	{
		TheFilter = MakeShareable(new OneEuroFilter<FVector>(Frequency, MinCutoff, Beta, DCutoff));

		bInitialized = true;
	}

	Value = NewValue;

	if (TheFilter.IsValid())
		FilteredValue = TheFilter->Filter(Value);
}

void FOneEuroFilterVector::GetValue(FVector& Raw, FVector& Filtered)
{
	Raw = Value;
	Filtered = FilteredValue;
}
void FOneEuroFilterVector::RefreshParameters()
{
	if (!TheFilter)
	{
		return;
	}
	TheFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
}
// FROTATOR

FOneEuroFilterRotator::FOneEuroFilterRotator(float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	FOneEuroFilterSettings(InFrequency, InMinCutoff, InBeta, InDCutoff);

	Value = FRotator::ZeroRotator;
	FilteredValue = FRotator::ZeroRotator;
	TheFilter = MakeShareable(new OneEuroFilter<FRotator>(Frequency, MinCutoff, Beta, DCutoff));
}

void FOneEuroFilterRotator::SetValue(FRotator NewValue)
{
	if (!bInitialized)
	{
		TheFilter = MakeShareable(new OneEuroFilter<FRotator>(Frequency, MinCutoff, Beta, DCutoff));

		bInitialized = true;
	}

	Value = NewValue;

	if (TheFilter.IsValid())
		FilteredValue = TheFilter->Filter(Value);
}

void FOneEuroFilterRotator::GetValue(FRotator& Raw, FRotator& Filtered)
{
	Raw = Value;
	Filtered = FilteredValue;
}
void FOneEuroFilterRotator::RefreshParameters()
{
	if (!TheFilter)
	{
		return;
	}
	TheFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
}

// FTRANSFORM

FOneEuroFilterTransform::FOneEuroFilterTransform(float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	FOneEuroFilterSettings(InFrequency, InMinCutoff, InBeta, InDCutoff);

	Value = FTransform::Identity;
	FilteredValue = FTransform::Identity;
}

void FOneEuroFilterTransform::SetValue(FTransform NewValue)
{
	if (!bInitialized)
	{
		TheLocationFilter = MakeShareable(new OneEuroFilter<FVector>(Frequency, MinCutoff, Beta, DCutoff));
		TheRotationFilter = MakeShareable(new OneEuroFilter<FQuat>(Frequency, MinCutoff, Beta, DCutoff));
		TheScaleFilter = MakeShareable(new OneEuroFilter<FVector>(Frequency, MinCutoff, Beta, DCutoff));

		bInitialized = true;
	}

	Value = NewValue;

	if (TheLocationFilter.IsValid())
		FilteredValue.SetLocation(TheLocationFilter->Filter(Value.GetLocation()));

	if (TheRotationFilter.IsValid())
		FilteredValue.SetRotation(TheRotationFilter->Filter(Value.Rotator().Quaternion()));

	if (TheScaleFilter.IsValid())
		FilteredValue.SetScale3D(TheScaleFilter->Filter(Value.GetScale3D()));
}

void FOneEuroFilterTransform::GetValue(bool bBypassScale, FTransform& Raw, FTransform& Filtered)
{
	Raw = Value;
	Filtered = FilteredValue;

	if (bBypassScale)
		Filtered.SetScale3D(Raw.GetScale3D());
}

void FOneEuroFilterTransform::RefreshParameters()
{
	TheLocationFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
	TheRotationFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
	TheScaleFilter->UpdateParams(Frequency, MinCutoff, Beta, DCutoff);
}