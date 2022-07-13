// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "JointOcclusionActor.generated.h"

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
	TArray<USceneCaptureComponent2D*> SceneCaptures;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void CountColoursInSceneCapture(const USceneCaptureComponent2D* SceneCapture);

};
