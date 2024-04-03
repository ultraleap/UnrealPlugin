// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapThrowing.h"
#include "LeapUtility.h"

// Sets default values for this component's properties
ULeapThrowing::ULeapThrowing() 
: ThrowSpeed(10)
, ThrownActor(nullptr)
, PlayerCameraManager(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Owner = GetOwner();

	if (Owner == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("Owner is nullptr in ULeapThrowing::ULeapThrowing()"));
		return;
	}

	// ...
}


// Called when the game starts
void ULeapThrowing::BeginPlay()
{
	Super::BeginPlay();

	// ...

	if (GEngine != nullptr)
	{
		LeapSubsystem = ULeapSubsystem::Get();
	}

	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapFrameMulti.AddUObject(this, &ULeapThrowing::OnLeapTrackingData);
		LeapSubsystem->OnLeapReleaseNative.AddUObject(this, &ULeapThrowing::OnReleased);
	}

	World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("GEngine or World is nullptr in BeginPlay"));
		return;
	}

	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	if (PlayerCameraManager == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("PlayerCameraManager is nullptr in BeginPlay"));
		return;
	}


}


// Called every frame
void ULeapThrowing::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULeapThrowing::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;

	for (const FLeapHandData& Hand : Hands)
	{
		switch (Hand.HandType)
		{
			case EHandType::LEAP_HAND_LEFT:
				UpdateQueue(LeftDataQ, Hand);
				break;
			case EHandType::LEAP_HAND_RIGHT:
				UpdateQueue(RightDataQ, Hand);
				break;
			default:
				break;
		}

	}
}

void ULeapThrowing::UpdateQueue(TCircularQueue<FTimedData>& DataQ, const FLeapHandData& Hand)
{
	FTimedData TimedData;

	LeapHand = Hand;

	TimedData.Position = Hand.Palm.Position;
	TimedData.Time = FDateTime::Now();
	DataQ.Enqueue(TimedData);

	//UE_LOG(UltraleapTrackingLog, Error, TEXT("TimedData.Position : %s"), *TimedData.Position.ToString());
}

void ULeapThrowing::Throw(TCircularQueue<FTimedData>& DataQ)
{
	const FTimedData* OldestData = DataQ.Peek();

	if (OldestData==nullptr)
	{
		return;
	}

	FTimedData NewestData;
	int32 Counter = 0;
	while (DataQ.Dequeue(NewestData) && Counter <=64)
	{
		Counter++;
	}
	
	
	//FVector Direction = NewestData.Position - PlayerCameraManager->GetCameraLocation();
	FVector Direction = LeapHand.Palm.Position - PlayerCameraManager->GetCameraLocation();
	
	FTimespan TimeSpan = NewestData.Time - OldestData->Time;


	
	UE_LOG(UltraleapTrackingLog, Error, TEXT("OldestData->Position : %s"), *OldestData->Position.ToString());
	UE_LOG(UltraleapTrackingLog, Error, TEXT("NewestData.Position : %s"), *NewestData.Position.ToString());

	float Speed = FVector::Dist(NewestData.Position, OldestData->Position) / TimeSpan.GetTotalSeconds();

	Direction.Normalize();

	float Dist = FVector::Dist(NewestData.Position, OldestData->Position);

	// FVector Force = Direction * Speed * ThrowSpeed;

	FVector Force = Direction * 1000 * Dist * ThrowSpeed;

	USceneComponent *SceneComponent = ThrownActor->GetRootComponent();

	if (SceneComponent)
	{
		UPrimitiveComponent *PrimitiveComponent = Cast<UPrimitiveComponent>(SceneComponent);
		if (PrimitiveComponent)
		{
			PrimitiveComponent->AddForceAtLocation(Force, ThrownActor->GetActorLocation());
		}
	}


	

	UE_LOG(UltraleapTrackingLog, Error, TEXT("Dist : %f"), Dist);

	LeftDataQ.Empty();
	RightDataQ.Empty();


	//ThrownActor->
}


void ULeapThrowing::OnReleased(
	AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName, bool IsLeft)
{
	if (ReleasedActor != nullptr)
	{
		ThrownActor = ReleasedActor;
		if (IsLeft)
		{
			Throw(LeftDataQ);	
		} 
		else 
		{
			Throw(RightDataQ);	
		}
		
	}
}


