// Fill out your copyright notice in the Description page of Project Settings.


#include "JointOcclusionActor.h"
#include "LeapComponent.h"
#include "FUltraleapCombinedDevice.h"

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

// Called every frame
void AJointOcclusionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

