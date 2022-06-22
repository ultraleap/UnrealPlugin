/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapTrackingPlugin.h"

#include "BodyStateBPLibrary.h"
#include "BodyStateDeviceConfig.h"
#include "FUltraleapTrackingInputDevice.h"
#include "IInputDeviceModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "LeapPlugin"

// this forces the assembly context to use the LeapC.dll in the Plugin
// without this, if OpenXR is enabled, the LeapC.dll in the OpenXR distribution will be loaded instead as it's
// loaded earlier on in the boot process
// This only works in the editor as Windows side by side expects the dll to be in either the same folder as the loading dll/exe
// or a direct sub folder, the folder structure for packaged apps means that this is not possible without breaking Unreal's folder
// structure.
#if PLATFORM_WINDOWS
#if WITH_EDITOR
#pragma comment(linker, \
	"\"/manifestdependency:type='win32' name='LeapC.dll' version='5.3.0.0' \
processorArchitecture='*' publicKeyToken='0000000000000000' language='*'\"")
#endif
#endif	  // PLATFORM_WINDOWS

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

	// early initialising works around device/input startup after begin play
	TSharedPtr<FGenericApplicationMessageHandler> DummyMessageHandler(new FGenericApplicationMessageHandler());
	CreateInputDevice(DummyMessageHandler.ToSharedRef());
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

FLeapStats FUltraleapTrackingPlugin::GetLeapStats(const FString& DeviceSerial)
{
	if (bActive)
	{
		return LeapInputDevice->GetStats(DeviceSerial);
	}
	else
	{
		return IUltraleapTrackingPlugin::GetLeapStats(DeviceSerial);
	}
}

void FUltraleapTrackingPlugin::SetOptions(const FLeapOptions& Options, const TArray<FString>& DeviceSerials)
{
	if (bActive)
	{
		LeapInputDevice->SetOptions(Options, DeviceSerials);
	}
}

FLeapOptions FUltraleapTrackingPlugin::GetOptions(const FString& DeviceSerial)
{
	if (bActive)
	{
		return LeapInputDevice->GetOptions(DeviceSerial);
	}
	else
	{
		return IUltraleapTrackingPlugin::GetOptions(DeviceSerial);
	}
}

void FUltraleapTrackingPlugin::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible,const FString& DeviceSerial)
{
	if (bActive)
	{
		LeapInputDevice->AreHandsVisible(LeftHandIsVisible, RightHandIsVisible, DeviceSerial);
	}
}

void FUltraleapTrackingPlugin::GetLatestFrameData(FLeapFrameData& OutData, const FString& DeviceSerial)
{
	// Copies data
	if (bActive)
	{
		LeapInputDevice->LatestFrame(OutData, DeviceSerial);
	}
}

void FUltraleapTrackingPlugin::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials)
{
	if (bActive)
	{
		LeapInputDevice->SetLeapPolicy(Flag, Enable, DeviceSerials);
	}
}
void FUltraleapTrackingPlugin::SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ,
	ELeapQuatSwizzleAxisB ToW, const TArray<FString>& DeviceSerials)
{
	if (bActive)
	{
		LeapInputDevice->SetSwizzles(ToX, ToY, ToZ, ToW, DeviceSerials);
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
ILeapConnector* FUltraleapTrackingPlugin::GetConnector()
{
	return LeapInputDevice->GetConnector();
}
void* FUltraleapTrackingPlugin::GetLeapHandle()
{
	void* NewLeapDLLHandle = nullptr;
	// Load LeapC DLL
	FString LeapCLibraryPath;

#if PLATFORM_WINDOWS
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("UltraleapTracking"));

	if (Plugin != nullptr)
	{
		FString BaseDir = Plugin->GetBaseDir();
		LeapCLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/LeapC.dll"));

		NewLeapDLLHandle = !LeapCLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LeapCLibraryPath) : nullptr;
	}
#endif	  // PLATFORM_WINDOWS

	if (NewLeapDLLHandle != nullptr)
	{
		UE_LOG(
			UltraleapTrackingLog, Log, TEXT("Engine plugin DLL found at %s"), *FPaths::ConvertRelativePathToFull(LeapCLibraryPath));
	}
	return NewLeapDLLHandle;
}

TSharedPtr<class IInputDevice> FUltraleapTrackingPlugin::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!LeapInputDevice.IsValid())
	{
		LeapInputDevice = MakeShareable(new FUltraleapTrackingInputDevice(InMessageHandler));
	}
	else
	{
		LeapInputDevice.Get()->SetMessageHandler(InMessageHandler);
		LeapInputDevice->PostEarlyInit();
	}
	bActive = true;

	// Add all the deferred components and empty it
	for (int i = 0; i < DeferredComponentList.Num(); i++)
	{
		AddEventDelegate(DeferredComponentList[i]);
	}
	DeferredComponentList.Empty();

	return LeapInputDevice;
}

IMPLEMENT_MODULE(FUltraleapTrackingPlugin, UltraleapTracking)

#undef LOCTEXT_NAMESPACE