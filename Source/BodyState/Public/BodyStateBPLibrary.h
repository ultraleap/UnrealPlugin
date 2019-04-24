// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "BodyStateInputInterface.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "BodyStateDeviceConfig.h"
#include "BodyStateBPLibrary.generated.h"

/* 
* Extra functions useful for animation rigging in BP
*/
UCLASS() 
class BODYSTATE_API UBodyStateBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "Body State Input")
	static int32 AttachDevice(const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate);

	static int32 AttachDeviceNative(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate);

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

	//Define mixing and update interfaces - this isn't ready yet, should it be called per skeleton or per bone?

	//BodyState Merging algorithm
	static bool AttachMergeAlgorithm(TFunction< void(UBodyStateSkeleton*, float)> InFunction);
};
