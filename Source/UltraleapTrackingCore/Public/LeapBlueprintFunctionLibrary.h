

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
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetLeapMode(ELeapMode Mode, ELeapTrackingFidelity Fidelity = ELeapTrackingFidelity::LEAP_NORMAL);

	/** Set global leap options */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetLeapOptions(const FLeapOptions& Options);

	/** Gets currently set global options */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void GetLeapOptions(FLeapOptions& OutOptions);

	/** Gets Leap read only stats such as api version, frame lookup and device information */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void GetLeapStats(FLeapStats& OutStats);

	/** Change leap policy */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);

	/** List the attached (plugged in) devices */
	UFUNCTION(BlueprintCallable, Category = "Leap Motion Functions")
	static void GetAttachedLeapDevices(TArray<FString>& Devices);

	/**Get the app version from the game.ini file */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static FString GetAppVersion();

	/** For debugging purposes only */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetDebugRotation(const FRotator& Rotator);
	// Debug functions, remove completely when no longer needed
	static void ShutdownLeap();

	static FRotator DebugRotator;
};