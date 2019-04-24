// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

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
	
	//Merging functions add/remove
	int32 AddMergingFunction(TFunction<void(UBodyStateSkeleton*, float)> InFunction);
	bool RemoveMergingFunction(int32 MergingFunctionId);
	void ClearMergingFunctions();

	/**
	* Call this function on all the attached devices. 
	*/
	void CallFunctionOnDevices(TFunction< void(const FBodyStateDevice&)> InFunction);

private:
	UBodyStateSkeleton* PrivateMergedSkeleton;

	TMap<IBodyStateInputRawInterface*, FBodyStateDevice> Devices;
	TMap<int32, IBodyStateInputRawInterface*> DeviceKeyMap;
	int32 DeviceIndex;
	double LastFrameTime;
	float DeltaTime;

	//Merging functions attached to skeletons
	TMap < int32, TFunction<void(UBodyStateSkeleton*, float)> > MergingFunctions;
	int32 MergingFunctionIndexCount;
};