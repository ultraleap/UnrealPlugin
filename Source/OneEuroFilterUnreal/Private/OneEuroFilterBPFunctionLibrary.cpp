// Fill out your copyright notice in the Description page of Project Settings.

#include "OneEuroFilterBPFunctionLibrary.h"

#include "OneEuroFilterUnreal.h"

// Blueprint functions to be used in conjunction with the structs above

// float ///////////
void UOneEuroFilterBPFunctionLibrary::GetFloatValue(FOneEuroFilterFloat InFloatFilter, float& Raw, float& Filtered)
{
	float TempRaw;
	float TempFiltered;
	InFloatFilter.GetValue(TempRaw, TempFiltered);

	Raw = TempRaw;
	Filtered = TempFiltered;

	return;
}

void UOneEuroFilterBPFunctionLibrary::SetFloatValue(UPARAM(ref) FOneEuroFilterFloat& InFloatFilter, float NewValue)
{
	InFloatFilter.SetValue(NewValue);
	return;
}

// Vector ////////////////
void UOneEuroFilterBPFunctionLibrary::GetVectorValue(FOneEuroFilterVector InVectorFilter, FVector& Raw, FVector& Filtered)
{
	FVector TempRaw;
	FVector TempFiltered;
	InVectorFilter.GetValue(TempRaw, TempFiltered);

	Raw = TempRaw;
	Filtered = TempFiltered;

	return;
}

void UOneEuroFilterBPFunctionLibrary::SetVectorValue(UPARAM(ref) FOneEuroFilterVector& InVectorFilter, FVector NewValue)
{
	InVectorFilter.SetValue(NewValue);
	return;
}

// Rotator ////////////
void UOneEuroFilterBPFunctionLibrary::GetRotatorValue(FOneEuroFilterRotator InRotatorFilter, FRotator& Raw, FRotator& Filtered)
{
	FRotator TempRaw;
	FRotator TempFiltered;
	InRotatorFilter.GetValue(TempRaw, TempFiltered);

	Raw = TempRaw;
	Filtered = TempFiltered;

	return;
}

void UOneEuroFilterBPFunctionLibrary::SetRotatorValue(UPARAM(ref) FOneEuroFilterRotator& InRotatorFilter, FRotator NewValue)
{
	InRotatorFilter.SetValue(NewValue);
	return;
}

// Transform ////////////
void UOneEuroFilterBPFunctionLibrary::GetTransformValue(
	FOneEuroFilterTransform InTransformFilter, FTransform& Raw, FTransform& Filtered, bool bBypassScale)
{
	FTransform TempRaw;
	FTransform TempFiltered;
	InTransformFilter.GetValue(bBypassScale, TempRaw, TempFiltered);

	Raw = TempRaw;
	Filtered = TempFiltered;

	return;
}

void UOneEuroFilterBPFunctionLibrary::SetTransformValue(UPARAM(ref) FOneEuroFilterTransform& InTransformFilter, FTransform NewValue)
{
	InTransformFilter.SetValue(NewValue);
	return;
}

void UOneEuroFilterBPFunctionLibrary::SetParametersTransform(
	UPARAM(ref) FOneEuroFilterTransform& InFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	InFilter.SetParameters(InFrequency, InMinCutoff, InBeta, InDCutoff);
}

void UOneEuroFilterBPFunctionLibrary::SetParametersFloat(
	UPARAM(ref) FOneEuroFilterFloat& InFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	InFilter.SetParameters(InFrequency, InMinCutoff, InBeta, InDCutoff);
}

void UOneEuroFilterBPFunctionLibrary::SetParametersVector(
	UPARAM(ref) FOneEuroFilterVector& InFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	InFilter.SetParameters(InFrequency, InMinCutoff, InBeta, InDCutoff);
}

void UOneEuroFilterBPFunctionLibrary::SetParametersRotator(
	UPARAM(ref) FOneEuroFilterRotator& InFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff)
{
	InFilter.SetParameters(InFrequency, InMinCutoff, InBeta, InDCutoff);
}
