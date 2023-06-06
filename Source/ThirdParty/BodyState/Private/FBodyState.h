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

#include "BodyStateSkeletonStorage.h"
#include "IBodyState.h"
class FBodyStateHMDDevice;

class FBodyState : public IBodyState
{
public:
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsActive();

	virtual bool IsInputReady() override;

	TSharedPtr<class FBodyStateInputDevice> BodyStateInputDevice;
	virtual TSharedPtr<class FBodyStateInputDevice> GetInputDevice()
	{
		return BodyStateInputDevice;
	}

	// Attach/Detach devices
	virtual int32 AttachDevice(
		const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate) override;
	virtual bool DetachDevice(int32 DeviceID) override;

	virtual UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID) override;

	virtual int32 AttachMergingFunctionForSkeleton(
		TFunction<void(UBodyStateSkeleton*, float)> InFunction, int32 SkeletonId = 0) override;
	virtual bool RemoveMergingFunction(int32 MergingFunctionId) override;

	virtual void AddBoneSceneListener(UBodyStateBoneComponent* Listener) override;
	virtual void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener) override;
	virtual bool GetAvailableDevices(TArray<FString>& DeviceSerials, TArray<int32>& DeviceIDs) override;
	virtual void SetupGlobalDeviceManager(IBodyStateDeviceManagerRawInterface* CallbackInterface) override;
	virtual int32 RequestCombinedDevice(const TArray<FString>& DeviceSerials, const EBSDeviceCombinerClass CombinerClass) override;
	virtual int32 GetDefaultDeviceID() override;

private:
	bool bActive = false;
	TSharedPtr<FBodyStateSkeletonStorage> SkeletonStorage;

	// Built-in devices
	TSharedPtr<FBodyStateHMDDevice> BSHMDDevice;
};