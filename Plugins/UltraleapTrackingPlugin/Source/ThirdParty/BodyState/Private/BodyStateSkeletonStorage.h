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

#include "BodyStateDevice.h"
class IBodyStateInputRawInterface;

/**
 * Holds all device/skeleton storage data
 */
class FBodyStateSkeletonStorage
{
public:
	FBodyStateSkeletonStorage();
	~FBodyStateSkeletonStorage();

	/**
	 * Add device, return unique id
	 */
	int32 AddDevice(const FBodyStateDevice& Device);

	/**
	 * Remove device by id. Return if operation removed the device, false if already removed
	 */
	bool RemoveDevice(int32 DeviceId);

	/**
	 * Usually used when cleaning up
	 */
	void RemoveAllDevices();

	/**
	 * Get the skeleton associated with the device
	 */
	UBodyStateSkeleton* SkeletonForDevice(int32 DeviceId);

	/**
	 * Return the merged skeleton
	 */
	UBodyStateSkeleton* MergedSkeleton();

	void UpdateMergeSkeletonData();
	void CallMergingFunctions();

	// Merging functions add/remove
	int32 AddMergingFunction(TFunction<void(UBodyStateSkeleton*, float)> InFunction);
	bool RemoveMergingFunction(int32 MergingFunctionId);
	void ClearMergingFunctions();

	/**
	 * Call this function on all the attached devices.
	 */
	void CallFunctionOnDevices(TFunction<void(const FBodyStateDevice&)> InFunction);

	bool GetAvailableDevices(TArray<FString>& DeviceSerials, TArray<int32>& DeviceIDs);
	
	void SetupGlobalDeviceManager(IBodyStateDeviceManagerRawInterface* CallbackInterface)
	{
		GlobalDeviceManager = CallbackInterface;
	}
	int32 RequestCombinedDevice(const TArray<FString>& DeviceSerials,
		const EBSDeviceCombinerClass CombinerClass)
	{
		if (GlobalDeviceManager)
		{
			return GlobalDeviceManager->RequestCombinedDevice(DeviceSerials, CombinerClass);
		}
		return -1;
	}
	int32 GetDefaultDeviceID()
	{
		if (GlobalDeviceManager)
		{
			return GlobalDeviceManager->GetDefaultDeviceID();
		}
		return 0;
	}
private:
	UBodyStateSkeleton* PrivateMergedSkeleton;

	TMap<IBodyStateInputRawInterface*, FBodyStateDevice> Devices;
	TMap<int32, IBodyStateInputRawInterface*> DeviceKeyMap;
	int32 DeviceIndex;
	double LastFrameTime;
	float DeltaTime;

	// Merging functions attached to skeletons
	TMap<int32, TFunction<void(UBodyStateSkeleton*, float)> > MergingFunctions;
	int32 MergingFunctionIndexCount;

	IBodyStateDeviceManagerRawInterface* GlobalDeviceManager = nullptr;
};