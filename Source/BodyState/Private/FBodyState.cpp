// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FBodyState.h"
#include "BodyStateSkeletonStorage.h"
#include "BodyStateBoneComponent.h"
#include "FBodyStateInputDevice.h"
#include "BodyStateHMDDevice.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "BodyState"

void FBodyState::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	// Custom module-specific init can go here.

	// IMPORTANT: This line registers our input device module with the engine.
	//	      If we do not register the input device module with the engine,
	//	      the engine won't know about our existence. Which means 
	//	      CreateInputDevice never gets called, which means the engine
	//	      will never try to poll for events from our custom input device.
	SkeletonStorage = MakeShareable(new FBodyStateSkeletonStorage());

	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}

void FBodyState::ShutdownModule()
{
	SkeletonStorage->CallFunctionOnDevices([](const FBodyStateDevice& Device)
	{
		Device.InputCallbackDelegate->OnDeviceDetach();
	});

	SkeletonStorage->RemoveAllDevices();

	// Unregister our input device module
	IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}


bool FBodyState::IsInputReady()
{
	return bActive;
}


int32 FBodyState::AttachDevice(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
{
	//Create Device
	FBodyStateDevice Device;
	Device.Config = Configuration;
	Device.InputCallbackDelegate = InputCallbackDelegate;

	return SkeletonStorage->AddDevice(Device);
}


bool FBodyState::DetachDevice(int32 DeviceID)
{
	return SkeletonStorage->RemoveDevice(DeviceID);
}


UBodyStateSkeleton* FBodyState::SkeletonForDevice(int32 DeviceID)
{
	return SkeletonStorage->SkeletonForDevice(DeviceID);
}

int32 FBodyState::AttachMergingFunctionForSkeleton(TFunction<void(UBodyStateSkeleton*, float)> InFunction, int32 SkeletonId /*= 0*/)
{
	//NB: skeleton id is ignored for now, skeleton is always the merged one atm
	return SkeletonStorage->AddMergingFunction(InFunction);
}

bool FBodyState::RemoveMergingFunction(int32 MergingFunctionId)
{
	return SkeletonStorage->RemoveMergingFunction(MergingFunctionId);
}

void FBodyState::AddBoneSceneListener(UBodyStateBoneComponent* Listener)
{
	//todo fill set listener transform
	BodyStateInputDevice->AddBoneSceneListener(Listener);
}

void FBodyState::RemoveBoneSceneListener(UBodyStateBoneComponent* Listener)
{
	//todo fill set listener transform
	BodyStateInputDevice->RemoveBoneSceneListener(Listener);
}

TSharedPtr< class IInputDevice > FBodyState::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	//UE_LOG(BodyStateLog, Log, TEXT("CreateInputDevice BodyState"));

	BodyStateInputDevice = MakeShareable(new FBodyStateInputDevice(InMessageHandler));

	//Forward storage pointer
	BodyStateInputDevice->SkeletonStorage = SkeletonStorage;

	bActive = true;

	//Setup additional inbuilt devices
	
	//HMD tracker
	FBodyStateDevice HMDDevice;
	
	BSHMDDevice = MakeShareable(new FBodyStateHMDDevice());
	HMDDevice.Config = BSHMDDevice->Config;
	HMDDevice.InputCallbackDelegate = BSHMDDevice.Get();

	BSHMDDevice->HMDDeviceIndex = SkeletonStorage->AddDevice(HMDDevice);

	return TSharedPtr< class IInputDevice >(BodyStateInputDevice);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBodyState, BodyState)