/*************************************************************************************************************************************
 *The MIT License(MIT)
 *
 *Copyright(c) 2016 Jan Kaniewski(Getnamo)
 *Modified work Copyright(C) 2019 - 2021 Ultraleap, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 *files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
 *merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions :
 *
 *The above copyright notice and this permission notice shall be included in all copies or
 *substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"

#include "BodyStateDeviceConfig.generated.h"

UENUM(BlueprintType)
enum EBodyStateDeviceInputType
{
	INERTIAL_INPUT_TYPE,			  // e.g. IMU with direct no external references like a Myo
	HMD_MOUNTED_INPUT_TYPE,			  // e.g. leap motion
	EXTERNAL_REFERENCE_INPUT_TYPE,	  // e.g. lighthouse
	MIXED_INPUT_TYPE				  // a mixture of previous types
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

	/** Serial no. of the device generating this input */
	UPROPERTY()
	FString DeviceSerial;
};