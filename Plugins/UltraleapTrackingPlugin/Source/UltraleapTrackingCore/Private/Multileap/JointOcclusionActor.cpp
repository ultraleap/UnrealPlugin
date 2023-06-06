/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "JointOcclusionActor.h"
#include "LeapComponent.h"
#include "FUltraleapCombinedDevice.h"
#include "Engine/TextureRenderTarget2D.h"



FLinearColor LerpLinearColor(const FLinearColor& Left, const FLinearColor& Right, const float Alpha)
{
	FLinearColor Ret(FMath::Lerp(Left.R, Right.R, Alpha),
	FMath::Lerp(Left.G, Right.G, Alpha), FMath::Lerp(Left.B, Right.B, Alpha));

	return Ret;
}
	// Sets default values
AJointOcclusionActor::AJointOcclusionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LeapComponent = CreateDefaultSubobject<ULeapComponent>(TEXT("Leap component"));

	static const bool DebugSimpleColours = true;
	static const bool UseLinearLerp = false;
	SetupColours(DebugSimpleColours, UseLinearLerp);
}
void AJointOcclusionActor::SetupColours(const bool DebugSimpleColours, const bool UseLinearLerp)
{
	SphereColoursLeft.Empty();
	SphereColoursRight.Empty();

	for (int i = 0; i < FUltraleapCombinedDevice::NumJointPositions; i++)
	{
		if (DebugSimpleColours)
		{
			SphereColoursLeft.Add(FColor::Red);
			SphereColoursRight.Add(FColor::Green);
		}
		else if (UseLinearLerp)
		{
			SphereColoursLeft.Add(
				LerpLinearColor(FColor::Red, FColor::Green, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
			SphereColoursRight.Add(
				LerpLinearColor(FColor::Yellow, FColor::Blue, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
		}
		else
		{
			SphereColoursLeft.Add(FLinearColor::LerpUsingHSV(
				FColor::Red, FColor::Green, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
			SphereColoursRight.Add(FLinearColor::LerpUsingHSV(
				FColor::Yellow, FColor::Blue, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
		}
	}
}
// Called when the game starts or when spawned
void AJointOcclusionActor::BeginPlay()
{
	Super::BeginPlay();
	
}
void AJointOcclusionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto Map : ColourCountMaps)
	{
		delete Map;
	}
	Super::EndPlay(EndPlayReason);
}
void AJointOcclusionActor::CountColoursInSceneCapture(
	const USceneCaptureComponent2D* SceneCapture, const FString& DeviceSerial, TMap<FLinearColor, int32>& ColourCountMap)
{
	auto RenderTarget = SceneCapture->TextureTarget->GameThread_GetRenderTargetResource();
	if (RenderTarget)
	{
		TArray<FLinearColor> Output;
		bool Success = RenderTarget->ReadLinearColorPixels(Output);
		
		if (Success)
		{
			ColourCountMap.Empty();
			
			for (const auto& Color : Output)
			{
				if (Color.IsAlmostBlack())
				{
					continue;
				}
				if (ColourCountMap.Find(Color))
				{
					ColourCountMap[Color] = ColourCountMap[Color]+1;
				}
				else
				{
					ColourCountMap.Add(Color, 1);
				}
			}
			// filter out odd pixels we don't care about
			auto ColourCountMapCopy = ColourCountMap;
			for (auto& KeyPair : ColourCountMapCopy)
			{
				if (KeyPair.Value < 2)
				{
					ColourCountMap.Remove(KeyPair.Key);
				}
			}
		}
	}
}
// for debugging only
bool AJointOcclusionActor::GetJointOcclusionConfidences(const FString& DeviceSerial, TArray<float>& Left, TArray<float>& Right)
{
	TArray<FString> DeviceSerials;

	DeviceToSceneCaptures.GetKeys(DeviceSerials);

	if (DeviceSerials.Num() == 0)
	{
		return false;
	}
	auto DeviceInterface = LeapComponent->GetCombinedDeviceBySerials(DeviceSerials);

	if (DeviceInterface == nullptr)
	{
		return false;
	}
	return DeviceInterface->GetJointOcclusionConfidences(DeviceSerial,  Left, Right);
}
void DebugPrintColourMap(const TMap<FLinearColor, int32>& ColourCountMap)
{
#if WITH_EDITOR
	if (GEngine)
	{
			for (auto& KeyPair : ColourCountMap)
			{
				FString Message;
				Message = FString::Printf(TEXT("ColourMap 1 %f %f %f %d"), KeyPair.Key.R, KeyPair.Key.G, KeyPair.Key.B,
	KeyPair.Value); GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Message);
			}
	}
#endif //WITH_EDITOR
}
void AJointOcclusionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TArray<FString> DeviceSerials;

	DeviceToSceneCaptures.GetKeys(DeviceSerials);

	if (DeviceSerials.Num() == 0)
	{
		return;
	}
	auto DeviceInterface = LeapComponent->GetCombinedDeviceBySerials(DeviceSerials);
	
	if (DeviceInterface == nullptr)
	{
		return;
	}

	int Index = 0;
	for (const auto& KeyValuePair : DeviceToSceneCaptures)
	{
		if (ColourCountMaps.Num() < (Index+1))
		{
			ColourCountMaps.Add(new FColourMap(KeyValuePair.Key));
		}
		// update device confidence values
		CountColoursInSceneCapture(KeyValuePair.Value,KeyValuePair.Key, ColourCountMaps[Index++]->ColourCountMap);
	}
	DeviceInterface->UpdateJointOcclusions(this);
}

