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

#include "BodyStateDeviceConfig.h"
#include "BodyStateInputInterface.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Skeleton/BodyStateSkeleton.h"

#include "BodyStateBPLibrary.generated.h"

/*
 * Extra functions useful for animation rigging in BP
 */
UCLASS()
class BODYSTATE_API UBodyStateBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "Body State Input")
	static int32 AttachDevice(
		const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate);

	static int32 AttachDeviceNative(
		const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate);

	UFUNCTION(BlueprintCallable, Category = "Body State Input")
	static bool DetachDevice(int32 DeviceID);

	/**
	 * Obtain the UBodyStateSkeleton attached to device or 0 if you want the merged skeleton
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Body State Input")
	static UBodyStateSkeleton* SkeletonForDevice(UObject* WorldContextObject, int32 DeviceID = 0);

	/** Convenience function for rigging*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Body State Input")
	static FTransform TransformForBoneNamedInAnimInstance(const FName& Bone, UAnimInstance* Instance);

	static UBodyStateSkeleton* RequestCombinedDevice(
		UObject* WorldContextObject, const TArray<FString>&, const EBSDeviceCombinerClass CombinerClass);

	static int32 GetDefaultDeviceID();
	// Global interface for device management, set to nullptr to clear
	static void SetupGlobalDeviceManager(IBodyStateDeviceManagerRawInterface* CallbackInterface);
	// Define mixing and update interfaces - this isn't ready yet, should it be called per skeleton or per bone?

	// BodyState Merging algorithm
	static bool AttachMergeAlgorithm(TFunction<void(UBodyStateSkeleton*, float)> InFunction);

	static bool GetAvailableDevices(TArray<FString>& DeviceSerials, TArray<int32>& DeviceIDs);

	static void AddDeviceChangeListener(IBodyStateDeviceChangeListener* Listener);
	static void RemoveDeviceChangeListener(IBodyStateDeviceChangeListener* Listener);
	static void OnDefaultDeviceChanged();
	static TArray<IBodyStateDeviceChangeListener*> DeviceChangeListeners;

};
