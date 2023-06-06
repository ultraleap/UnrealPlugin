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
#include "UObject/Interface.h"

#include "BodyStateInputInterface.generated.h"

// For non-uobjects
class BODYSTATE_API IBodyStateInputRawInterface
{
public:
	virtual void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton) = 0;
	virtual void OnDeviceDetach() = 0;
};
UENUM(BlueprintType)
enum EBSDeviceCombinerClass
{
	BS_DEVICE_COMBINER_UNKNOWN,
	BS_DEVICE_COMBINER_CONFIDENCE,
	BS_DEVICE_COMBINER_ANGULAR
	// add your custom classes here and add them to the class factory
};
class BODYSTATE_API IBodyStateDeviceManagerRawInterface
{
public:
	// return bodystate device ID for combined device
	// device may already exist if requested elsewhere, created if not
	virtual int32 RequestCombinedDevice(const TArray<FString>& DeviceSerials,const EBSDeviceCombinerClass CombinerClass) = 0;
	// get the default bodystate device ID
	// this can be different depending on whether OpenXR global mode is set or not
	virtual int32 GetDefaultDeviceID() = 0;
};

class BODYSTATE_API IBodyStateDeviceChangeListener
{
public:
	virtual void OnDeviceAdded(const FString& DeviceSerial, const uint32 DeviceID) = 0;
	virtual void OnDeviceRemoved(const uint32 DeviceID) = 0;
	virtual void OnDefaultDeviceChanged() = 0;
};
// for uobject and bps
UINTERFACE(Blueprintable, MinimalAPI)
class UBodyStateInputInterface : public UInterface
{
	GENERATED_BODY()
};

class BODYSTATE_API IBodyStateInputInterface
{
	GENERATED_BODY()

public:
	/* Requests your device to update the skeleton. You can use this BS polling method or push updates to your skeleton*/
	UFUNCTION(BlueprintNativeEvent, Category = "Body State Poll Interface")
	void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton);	 // todo: define
};
