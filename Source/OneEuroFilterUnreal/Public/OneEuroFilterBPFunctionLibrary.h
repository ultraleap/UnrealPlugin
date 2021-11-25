// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OneEuroFilterBlueprintDatatypes.h"

#include "OneEuroFilterBPFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class ONEEUROFILTERUNREAL_API UOneEuroFilterBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// float ///////////
	UFUNCTION(BlueprintPure, Category = "One Euro Filter")
	static void GetFloatValue(FOneEuroFilterFloat InFloatFilter, float& Raw, float& Filtered);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetFloatValue(UPARAM(ref) FOneEuroFilterFloat& InFloatFilter, float NewValue);

	// Vector ////////////////
	UFUNCTION(BlueprintPure, Category = "One Euro Filter")
	static void GetVectorValue(FOneEuroFilterVector InVectorFilter, FVector& Raw, FVector& Filtered);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetVectorValue(UPARAM(ref) FOneEuroFilterVector& InVectorFilter, FVector NewValue);

	// Rotator ////////////
	UFUNCTION(BlueprintPure, Category = "One Euro Filter")
	static void GetRotatorValue(FOneEuroFilterRotator InRotatorFilter, FRotator& Raw, FRotator& Filtered);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetRotatorValue(UPARAM(ref) FOneEuroFilterRotator& InRotatorFilter, FRotator NewValue);

	// Transform ////////////
	UFUNCTION(BlueprintPure, Category = "One Euro Filter")
	static void GetTransformValue(
		FOneEuroFilterTransform InTransformFilter, FTransform& Raw, FTransform& Filtered, bool bBypassScale = true);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetTransformValue(UPARAM(ref) FOneEuroFilterTransform& InTransformFilter, FTransform NewValue);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetParametersTransform(UPARAM(ref) FOneEuroFilterTransform& InTransformFilter, float InFrequency, float InMinCutoff,
		float InBeta, float InDCutoff);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetParametersFloat(
		UPARAM(ref) FOneEuroFilterFloat& InFloatFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetParametersVector(
		UPARAM(ref) FOneEuroFilterVector& InVectorFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff);

	UFUNCTION(BlueprintCallable, Category = "One Euro Filter")
	static void SetParametersRotator(
		UPARAM(ref) FOneEuroFilterRotator& InRotatorFilter, float InFrequency, float InMinCutoff, float InBeta, float InDCutoff);
};
