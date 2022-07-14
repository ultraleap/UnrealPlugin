// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "JointOcclusionActor.generated.h"

class FColourMap
{
public:
	FColourMap(const FString DeviceSerialIn)
	{
		DeviceSerial = DeviceSerialIn;
	}

	TMap<FLinearColor, int32> ColourCountMap;
	FString DeviceSerial;
};
UCLASS()
class AJointOcclusionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJointOcclusionActor();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Leap Devices - Joint Occlusion")
	class ULeapComponent* LeapComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Leap Devices - Joint Occlusion")
	TArray<FLinearColor> SphereColoursLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Leap Devices - Joint Occlusion")
	TArray<FLinearColor> SphereColoursRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Leap Devices - Joint Occlusion")
	TMap<FString, USceneCaptureComponent2D*> DeviceToSceneCaptures;

	
	UFUNCTION(BlueprintCallable, Category = "Leap Devices - Joint Occlusion")
	bool GetJointOcclusionConfidences(const FString& DeviceSerial, TArray<float>& Left, TArray<float>& Right);
	
	const TArray<FColourMap*>& GetColourCountMaps()
	{
		return ColourCountMaps;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void CountColoursInSceneCapture(
		const USceneCaptureComponent2D* SceneCapture, const FString& DeviceSerial, TMap<FLinearColor, int32>& ColourCountMap);
	TArray<FColourMap*> ColourCountMaps;
};
