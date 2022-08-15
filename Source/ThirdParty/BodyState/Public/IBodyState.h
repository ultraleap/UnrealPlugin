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
#include "IInputDeviceModule.h"
#include "Runtime/Core/Public/Modules/ModuleInterface.h"

class IBodyStateInputRawInterface;
class UBodyStateBoneComponent;

class BODYSTATE_API IBodyState : public IInputDeviceModule
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
		return FModuleManager::LoadModuleChecked<IBodyState>("BodyState");
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
	virtual bool IsInputReady()
	{
		return false;
	}

	// Add/remove device
	virtual int32 AttachDevice(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
	{
		return -1;
	}
	virtual bool DetachDevice(int32 DeviceID)
	{
		return false;
	}

	// Fetch
	virtual class UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID)
	{
		return nullptr;
	}

	// Merging

	/**
	 * Attaches a merging function to the specified skeleton. This is called after update input is called for that skeleton
	 * A skeleton id of 0 specifies the merged device skeleton.
	 * @returns function ID. use this ID to remove the merging function
	 */
	virtual int32 AttachMergingFunctionForSkeleton(TFunction<void(UBodyStateSkeleton*, float)> InFunction, int32 SkeletonId = 0)
	{
		return -1;
	}
	virtual bool RemoveMergingFunction(int32 MergingFunctionId)
	{
		return false;
	};

	// Copying movement
	virtual void AddBoneSceneListener(UBodyStateBoneComponent* Listener){};
	virtual void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener){};

	virtual bool GetAvailableDevices(TArray<FString>& DeviceSerials, TArray<int32>& DeviceIDs)
	{
		return false;
	}

	// Global interface for device management, set to nullptr to clear
	virtual void SetupGlobalDeviceManager(IBodyStateDeviceManagerRawInterface* CallbackInterface)
	{
	}

	virtual int32 RequestCombinedDevice(const TArray<FString>& DeviceSerials, const EBSDeviceCombinerClass CombinerClass)
	{
		return -1;
	}

	virtual int32 GetDefaultDeviceID()
	{
		return 0;
	}
};