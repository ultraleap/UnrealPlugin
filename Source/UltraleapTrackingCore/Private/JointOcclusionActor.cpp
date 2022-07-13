// Fill out your copyright notice in the Description page of Project Settings.


#include "JointOcclusionActor.h"
#include "LeapComponent.h"
#include "FUltraleapCombinedDevice.h"
#include "Engine/TextureRenderTarget2D.h"




// Sets default values
AJointOcclusionActor::AJointOcclusionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LeapComponent = CreateDefaultSubobject<ULeapComponent>(TEXT("Leap component"));

	for (int i = 0; i < FUltraleapCombinedDevice::NumJointPositions; i++)
	{
		SphereColoursLeft.Add(FLinearColor::LerpUsingHSV(FColor::Red, FColor::Green, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
		SphereColoursRight.Add(FLinearColor::LerpUsingHSV(FColor::Yellow, FColor::Blue, (float) i / (float) FUltraleapCombinedDevice::NumJointPositions));
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
			const auto SizeX = RenderTarget->GetSizeX();
			const auto SizeY = RenderTarget->GetSizeY();

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
		}
	}
}
// Called every frame
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
	for (const auto KeyValuePair : DeviceToSceneCaptures)
	{
		if (ColourCountMaps.Num() < (Index-1))
		{
			ColourCountMaps.Add(new FColourMap(KeyValuePair.Key));
		}
		// update device confidence values
		CountColoursInSceneCapture(KeyValuePair.Value,KeyValuePair.Key, ColourCountMaps[Index++]->ColourCountMap);
	}
	DeviceInterface->UpdateJointOcclusions(this);
}

