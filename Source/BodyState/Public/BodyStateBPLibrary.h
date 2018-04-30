// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "BodyStateInputInterface.h"
#include "BodyStateSkeleton.h"
#include "BodyStateDeviceConfig.h"
#include "BodyStateBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
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

	//Define mixing and update interfaces - this isn't ready yet, should it be called per skeleton or per bone?

	//BodyState Merging algorithm
	static bool AttachMergeAlgorithm(TFunction< void(UBodyStateSkeleton*)> InFunction);
};
