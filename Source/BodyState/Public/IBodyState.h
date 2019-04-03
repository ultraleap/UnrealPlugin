// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/Modules/ModuleInterface.h"
#include "BodyStateDeviceConfig.h"
#include "BodyStateInputInterface.h"
#include "IInputDeviceModule.h"

class IBodyStateInputRawInterface;
class UBodyStateBoneComponent;

class BODYSTATE_API  IBodyState : public IInputDeviceModule
{
public:
	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline IBodyState& Get()
	{
		return FModuleManager::LoadModuleChecked< IBodyState >("BodyState");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BodyState");
	}

	/** Public Module methods*/
	virtual bool IsInputReady() { return false; }

	//Add/remove device
	virtual int32 AttachDevice(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate) { return -1;}
	virtual bool DetachDevice(int32 DeviceID) { return false; }

	//Fetch
	virtual class UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID) { return nullptr; }

	//Merging

	/**
	* Attaches a merging function to the specified skeleton. This is called after update input is called for that skeleton
	* A skeleton id of 0 specifies the merged device skeleton.
	* @returns function ID. use this ID to remove the merging function 
	*/
	virtual int32 AttachMergingFunctionForSkeleton(TFunction<void(UBodyStateSkeleton*, float)> InFunction, int32 SkeletonId = 0) { return -1; }
	virtual bool RemoveMergingFunction(int32 MergingFunctionId) { return false; };

	//Copying movement
	virtual void AddBoneSceneListener(UBodyStateBoneComponent* Listener) { };
	virtual void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener) { };
};