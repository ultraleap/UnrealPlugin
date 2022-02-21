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

FRotator ULeapBlueprintFunctionLibrary::DebugRotator;
ULeapBlueprintFunctionLibrary::ULeapBlueprintFunctionLibrary(const class FObjectInitializer& Initializer) : Super(Initializer)
{
}

void ULeapBlueprintFunctionLibrary::SetLeapMode(ELeapMode InMode, ELeapTrackingFidelity InTrackingFidelity)
{
	FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions();

	Options.Mode = InMode;
	Options.TrackingFidelity = InTrackingFidelity;

	SetLeapOptions(Options);
}

void ULeapBlueprintFunctionLibrary::SetLeapOptions(const FLeapOptions& InOptions)
{
	IUltraleapTrackingPlugin::Get().SetOptions(InOptions);
}

void ULeapBlueprintFunctionLibrary::GetLeapOptions(FLeapOptions& OutOptions)
{
	OutOptions = IUltraleapTrackingPlugin::Get().GetOptions();
}

void ULeapBlueprintFunctionLibrary::GetLeapStats(FLeapStats& OutStats)
{
	OutStats = IUltraleapTrackingPlugin::Get().GetLeapStats();
}

void ULeapBlueprintFunctionLibrary::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	IUltraleapTrackingPlugin::Get().SetLeapPolicy(Flag, Enable);
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