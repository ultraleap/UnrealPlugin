// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BodyStateDeviceConfig.generated.h"

UENUM(BlueprintType)
enum EBodyStateDeviceInputType
{
	INERTIAL_INPUT_TYPE,			//e.g. IMU with direct no external references like a Myo
	HMD_MOUNTED_INPUT_TYPE,			//e.g. leap motion
	EXTERNAL_REFERENCE_INPUT_TYPE,	//e.g. lighthouse
	MIXED_INPUT_TYPE				//a mixture of previous types
};


USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateDeviceConfig
{
	GENERATED_USTRUCT_BODY()

	FBodyStateDeviceConfig();

	/** Name of the device generating this input */
	UPROPERTY()
	FString DeviceName;

	/** Input type this device uses */
	UPROPERTY()
	TEnumAsByte<EBodyStateDeviceInputType> InputType;

	/** Any specific tracking tags you may wish to expose to various systems, e.g. Finger Hand Tracking, Full Body Tracking*/
	UPROPERTY()
	TArray<FString> TrackingTags;
};