/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UltraleapTrackingData.h"

#include "LeapBlueprintFunctionLibrary.generated.h"

/**
 * Useful global blueprint functions for Ultraleap Tracking
 */
UCLASS()
class ULeapBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Set basic global leap tracking options */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions", meta = (AutoCreateRefTerm = "DeviceSerials"))
	static void SetLeapMode(ELeapMode Mode, const TArray<FString>& DeviceSerials, ELeapTrackingFidelity Fidelity = ELeapTrackingFidelity::LEAP_NORMAL);

	/** Set global leap options */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions", meta = (AutoCreateRefTerm = "DeviceSerials"))
	static void SetLeapOptions(const FLeapOptions& Options, const TArray<FString>& DeviceSerials);

	/** Gets currently set global options */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions", meta = (AutoCreateRefTerm = "DeviceSerial"))
	static void GetLeapOptions(FLeapOptions& OutOptions, const FString& DeviceSerial);

	/** Gets Leap read only stats such as api version, frame lookup and device information */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions", meta = (AutoCreateRefTerm = "DeviceSerial"))
	static void GetLeapStats(FLeapStats& OutStats, const FString& DeviceSerial);

	/** Change leap policy */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions", meta = (AutoCreateRefTerm = "DeviceSerials"))
	static void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials);

	/** List the attached (plugged in) devices */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void GetAttachedLeapDevices(TArray<FString>& Devices);

	/**Get the app version from the game.ini file */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static FString GetAppVersion();

	/** For debugging purposes only */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetDebugRotation(const FRotator& Rotator);

	/** Angle between vectors - Equivalent of Unity's Vector3.Angle*/
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static float AngleBetweenVectors(const FVector& A, const FVector& B);

	// Debug functions, remove completely when no longer needed
	static void ShutdownLeap();

	static FRotator DebugRotator;
};