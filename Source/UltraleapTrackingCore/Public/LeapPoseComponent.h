// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LeapSaveGame.h"

#include "LeapPoseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRALEAPTRACKING_API ULeapPoseComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULeapPoseComponent();

	

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void GetPose(FString PoseName, FPoseSnapshot& Pose);

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void SaveNewPose(FString PoseName);

	USkeletalMeshComponent *Hand;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
