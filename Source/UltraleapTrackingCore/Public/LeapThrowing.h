// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LeapSubsystem.h"
#include "Containers/CircularQueue.h"
#include "Kismet/GameplayStatics.h"
#include "LeapThrowing.generated.h"

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FTimedData
{
	GENERATED_USTRUCT_BODY()

	FVector Position;
	FDateTime Time;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRALEAPTRACKING_API ULeapThrowing : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULeapThrowing();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap Throw")
	float ThrowSpeed;

private: 
	ULeapSubsystem* LeapSubsystem;
	AActor* Owner;
	AActor* ThrownActor;
	APlayerCameraManager* PlayerCameraManager;
	UWorld* World;

	void OnLeapTrackingData(const FLeapFrameData& Frame);

	TCircularQueue<FTimedData> LeftDataQ{64};
	TCircularQueue<FTimedData> RightDataQ{64};

	void UpdateQueue(TCircularQueue<FTimedData>& DataQ, const FLeapHandData& Hand);
	void Throw(TCircularQueue<FTimedData>& DataQ);

	void OnReleased(
		AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName, bool IsLeft = true);

	 FLeapHandData LeapHand;
};
