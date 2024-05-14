/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "LeapBlueprintFunctionLibrary.h"

#include "IUltraleapTrackingPlugin.h"
#include "Misc/ConfigCacheIni.h"
#include "LeapSubsystem.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include <android/log.h>
#endif

FRotator ULeapBlueprintFunctionLibrary::DebugRotator;
ULeapBlueprintFunctionLibrary::ULeapBlueprintFunctionLibrary(const class FObjectInitializer& Initializer) : Super(Initializer)
{
}

void ULeapBlueprintFunctionLibrary::SetLeapMode(
	ELeapMode InMode, const TArray<FString>& DeviceSerials, ELeapTrackingFidelity InTrackingFidelity)
{
	// if empty, default to single device logic
	if (!DeviceSerials.Num())
	{
		FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions("");

		Options.Mode = InMode;
		Options.TrackingFidelity = InTrackingFidelity;

		SetLeapOptions(Options, DeviceSerials);
	}
	else
	{
		for (auto DeviceSerial : DeviceSerials)
		{
			FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions(DeviceSerial);

			Options.Mode = InMode;
			Options.TrackingFidelity = InTrackingFidelity;

			TArray<FString> SingleDevice;
			SingleDevice.Add(DeviceSerial);
			SetLeapOptions(Options, SingleDevice);
		}
	}
}

void ULeapBlueprintFunctionLibrary::SetLeapOptions(const FLeapOptions& InOptions, const TArray<FString>& DeviceSerials)
{
	if (ULeapSubsystem* LeapSubsystem = ULeapSubsystem::Get())
	{
		LeapSubsystem->SetUseOpenXR(InOptions.bUseOpenXRAsSource);
	}
	IUltraleapTrackingPlugin::Get().SetOptions(InOptions, DeviceSerials);
}

void ULeapBlueprintFunctionLibrary::GetLeapOptions(FLeapOptions& OutOptions, const FString& DeviceSerial)
{
	OutOptions = IUltraleapTrackingPlugin::Get().GetOptions(DeviceSerial);
}

void ULeapBlueprintFunctionLibrary::GetLeapStats(FLeapStats& OutStats, const FString& DeviceSerial)
{
	OutStats = IUltraleapTrackingPlugin::Get().GetLeapStats(DeviceSerial);
}

void ULeapBlueprintFunctionLibrary::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials)
{
	IUltraleapTrackingPlugin::Get().SetLeapPolicy(Flag, Enable, DeviceSerials);
}
void ULeapBlueprintFunctionLibrary::GetAttachedLeapDevices(TArray<FString>& Devices)
{
	IUltraleapTrackingPlugin::Get().GetAttachedDevices(Devices);
}
FString ULeapBlueprintFunctionLibrary::GetAppVersion()
{
	FString AppVersion;
	if (GConfig)
	{
		GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), AppVersion, GGameIni);
	}
	return AppVersion;
}
// Debug Functions
void ULeapBlueprintFunctionLibrary::ShutdownLeap()
{
	IUltraleapTrackingPlugin::Get().ShutdownLeap();
}
void ULeapBlueprintFunctionLibrary::SetDebugRotation(const FRotator& Rotator)
{
	DebugRotator = Rotator;
}
float ULeapBlueprintFunctionLibrary::AngleBetweenVectors(const FVector& A, const FVector& B)
{
	float AngleCosine = FVector::DotProduct(A, B) / (A.Size() * B.Size());
	float AngleRadians = FMath::Acos(AngleCosine);
	return FMath::RadiansToDegrees(AngleRadians);
}

void ULeapBlueprintFunctionLibrary::SetLeapDeviceHints(const TArray<FString>& DeviceSerials, const TArray<FString>& Hints)
{
	// if empty, default to single device logic
	if (!DeviceSerials.Num())
	{
		FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions("");
		if (Options.LeapHints != Hints)
		{
			Options.LeapHints = Hints;
			SetLeapOptions(Options, DeviceSerials);
		}
	}
	else
	{
		for (FString DeviceSerial : DeviceSerials)
		{
			FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions(DeviceSerial);
			if (Options.LeapHints != Hints)
			{
				Options.LeapHints = Hints;
				TArray<FString> SingleDevice;
				SingleDevice.Add(DeviceSerial);
				SetLeapOptions(Options, SingleDevice);
			}
		}
	}
}

void ULeapBlueprintFunctionLibrary::AddLeapDeviceHint(const TArray<FString>& DeviceSerials, const FString& Hint)
{
	if (!DeviceSerials.Num())
	{
		FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions("");
		if (!Options.LeapHints.Contains(Hint))
		{
			Options.LeapHints.Add(Hint);
			SetLeapOptions(Options, DeviceSerials);
		}
	}
	else
	{
		for (FString DeviceSerial : DeviceSerials)
		{
			FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions(DeviceSerial);
			if (!Options.LeapHints.Contains(Hint))
			{
				Options.LeapHints.Add(Hint);
				TArray<FString> SingleDevice;
				SingleDevice.Add(DeviceSerial);
				SetLeapOptions(Options, SingleDevice);
			}
		}
	}
}

void ULeapBlueprintFunctionLibrary::RemoveLeapDeviceHint(const TArray<FString>& DeviceSerials, const FString& Hint)
{
	
	if (!DeviceSerials.Num())
	{
		FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions("");
		if (Options.LeapHints.Contains(Hint))
		{
			Options.LeapHints.Remove(Hint);
			SetLeapOptions(Options, DeviceSerials);
		}
	}
	else
	{
		for (FString DeviceSerial : DeviceSerials)
		{
			FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions(DeviceSerial);
			if (Options.LeapHints.Contains(Hint))
			{
				Options.LeapHints.Remove(Hint);
				TArray<FString> SingleDevice;
				SingleDevice.Add(DeviceSerial);
				SetLeapOptions(Options, SingleDevice);
			}
		}
	}
}

void ULeapBlueprintFunctionLibrary::BindTrackingServiceAndroid()
{
#if PLATFORM_ANDROID
	// Binding the tracking service
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method =
			FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_Bind", "()V", false);
		if (Method)
		{
			UE_LOG(UltraleapTrackingLog, Log, TEXT("UltraleapTracking: calling AndroidThunkJava_Bind"));
			FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
		}
		else
		{
			UE_LOG(UltraleapTrackingLog, Error, TEXT("UltraleapTracking: could not call AndroidThunkJava_Bind invalid Method"));
		}
	}
#endif
}

void ULeapBlueprintFunctionLibrary::UnbindTrackingServiceAndroid()
{
#if PLATFORM_ANDROID
	//  Unbind the tracking service
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method =
			FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_Unbind", "()V", false);
		if (Method)
		{
			UE_LOG(UltraleapTrackingLog, Log, TEXT("UltraleapTracking: calling AndroidThunkJava_Unbind"));
			FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
		}
		else
		{
			UE_LOG(UltraleapTrackingLog, Error, TEXT("UltraleapTracking: could not call AndroidThunkJava_Unbind invalid Method"));
		}
	}
#endif
}