// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapMotionData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LeapBlueprintFunctionLibrary.generated.h"

/**
* Useful global blueprint functions for Leap Motion
*/
UCLASS()
class ULeapBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Set basic global leap tracking options */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void SetLeapMode(ELeapMode Mode, ELeapTrackingFidelity Fidelity = ELeapTrackingFidelity::LEAP_NORMAL);

	/** Set global leap options */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void SetLeapOptions(const FLeapOptions& Options);

	/** Gets currently set global options */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void GetLeapOptions(FLeapOptions& OutOptions);

	/** Gets Leap read only stats such as api version, frame lookup and device information */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void GetLeapStats(FLeapStats& OutStats);

	/** Change leap policy */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);

	//Debug functions, remove completely when no longer needed
	static void ShutdownLeap();
};