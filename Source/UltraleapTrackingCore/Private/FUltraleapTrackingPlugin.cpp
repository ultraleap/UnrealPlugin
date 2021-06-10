// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "FUltraleapTrackingPlugin.h"

#include "BodyStateBPLibrary.h"
#include "BodyStateDeviceConfig.h"
#include "FUltraleapTrackingInputDevice.h"
#include "IInputDeviceModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "LeapPlugin"

void FUltraleapTrackingPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	// Custom module-specific init can go here.

	// IMPORTANT: This line registers our input device module with the engine.
	//	      If we do not register the input device module with the engine,
	//	      the engine won't know about our existence. Which means
	//	      CreateInputDevice never gets called, which means the engine
	//	      will never try to poll for events from our custom input device.

	// Load the dll from appropriate location
	LeapDLLHandle = GetLeapHandle();

	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);

	// Get and display our plugin version in the log for debugging
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("UltraleapTracking"));
	const FPluginDescriptor& PluginDescriptor = Plugin->GetDescriptor();
	UE_LOG(UltraleapTrackingLog, Log, TEXT("Leap Plugin started v%s"), *PluginDescriptor.VersionName);
}

void FUltraleapTrackingPlugin::ShutdownModule()
{
	UE_LOG(UltraleapTrackingLog, Log, TEXT("Leap Plugin shutdown."));

	if (LeapDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(LeapDLLHandle);
		LeapDLLHandle = nullptr;
	}

	// Unregister our input device module
	IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}

void FUltraleapTrackingPlugin::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->AddEventDelegate(EventDelegate);
	}
	else
	{
		// Delay load it until it is ready
		DeferredComponentList.Add((ULeapComponent*) EventDelegate);
	}
}

void FUltraleapTrackingPlugin::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->RemoveEventDelegate(EventDelegate);
	}
}

FLeapStats FUltraleapTrackingPlugin::GetLeapStats()
{
	if (bActive)
	{
		return LeapInputDevice->GetStats();
	}
	else
	{
		return IUltraleapTrackingPlugin::GetLeapStats();
	}
}

void FUltraleapTrackingPlugin::SetOptions(const FLeapOptions& Options)
{
	if (bActive)
	{
		LeapInputDevice->SetOptions(Options);
	}
}

FLeapOptions FUltraleapTrackingPlugin::GetOptions()
{
	if (bActive)
	{
		return LeapInputDevice->GetOptions();
	}
	else
	{
		return IUltraleapTrackingPlugin::GetOptions();
	}
}

void FUltraleapTrackingPlugin::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	if (bActive)
	{
		LeapInputDevice->AreHandsVisible(LeftHandIsVisible, RightHandIsVisible);
	}
}

void FUltraleapTrackingPlugin::GetLatestFrameData(FLeapFrameData& OutData)
{
	// Copies data
	if (bActive)
	{
		LeapInputDevice->LatestFrame(OutData);
	}
}

void FUltraleapTrackingPlugin::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	if (bActive)
	{
		LeapInputDevice->SetLeapPolicy(Flag, Enable);
	}
}
void FUltraleapTrackingPlugin::GetAttachedDevices(TArray<FString>& Devices)
{
	if (bActive)
	{
		Devices = LeapInputDevice->GetAttachedDevices();
	}
}
void FUltraleapTrackingPlugin::ShutdownLeap()
{
	if (bActive)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Shutting down leap from command."));
		LeapInputDevice->ShutdownLeap();
	}
}

void* FUltraleapTrackingPlugin::GetLeapHandle()
{
	void* NewLeapDLLHandle = nullptr;

#if PLATFORM_WINDOWS
#if PLATFORM_64BITS
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/UltraleapTracking/Win64"));
#else
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/UltraleapTracking/Win32"));
#endif
	FPlatformProcess::PushDllDirectory(*BinariesPath);
	NewLeapDLLHandle = FPlatformProcess::GetDllHandle(*(BinariesPath / "LeapC.dll"));
	FPlatformProcess::PopDllDirectory(*BinariesPath);
#endif

	if (NewLeapDLLHandle != nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Engine plugin DLL found at %s"),
			*FPaths::ConvertRelativePathToFull(BinariesPath / "LeapC.dll"));
	}
	return NewLeapDLLHandle;
}

TSharedPtr<class IInputDevice> FUltraleapTrackingPlugin::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	FUltraleapTrackingPlugin::LeapInputDevice = MakeShareable(new FUltraleapTrackingInputDevice(InMessageHandler));

	bActive = true;

	// Add all the deferred components and empty it
	for (int i = 0; i < DeferredComponentList.Num(); i++)
	{
		AddEventDelegate(DeferredComponentList[i]);
	}
	DeferredComponentList.Empty();

	return TSharedPtr<class IInputDevice>(LeapInputDevice);
}

IMPLEMENT_MODULE(FUltraleapTrackingPlugin, UltraleapTracking)

#undef LOCTEXT_NAMESPACE