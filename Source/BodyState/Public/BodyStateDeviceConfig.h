// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"
#include "Engine.h"
#include "BodyStateDeviceConfig.generated.h"

UENUM(BlueprintType)
enum EBodyStateDeviceInputType
{
	INERTIAL_INPUT_TYPE,			//e.g. IMU with direct no external references like a Myo
	HMD_MOUNTED_INPUT_TYPE,		//e.g. leap motion
	EXTERNAL_REFERENCE_INPUT_TYPE,	//e.g. lighthouse
	MIXED_INPUT_TYPE
};


USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateDeviceConfig
{
	GENERATED_USTRUCT_BODY()

	FBodyStateDeviceConfig();

	UPROPERTY()
	FString DeviceName;

	UPROPERTY()
	TEnumAsByte<EBodyStateDeviceInputType> InputType;
};