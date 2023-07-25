// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapPoseComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
ULeapPoseComponent::ULeapPoseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void ULeapPoseComponent::BeginPlay()
{
	Super::BeginPlay();
	TArray<UActorComponent*> Hands = GetOwner()->GetComponentsByClass(USkeletalMeshComponent::StaticClass());

	for (UActorComponent* HandTmp : Hands)
	{
		FName HandName = HandTmp->GetFName();
		FString StrName = HandName.ToString();
			
		if (StrName.Contains(TEXT("Right")))
		{
			Hand = Cast<USkeletalMeshComponent>(HandTmp);
		}
	}

	// ...
	
}


// Called every frame
void ULeapPoseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULeapPoseComponent::SaveNewPose(FString PoseName, FPoseSnapshot Pose)
{
	if (ULeapSaveGame* SaveGameInstance = Cast<ULeapSaveGame>(UGameplayStatics::CreateSaveGameObject(ULeapSaveGame::StaticClass())))
	{
		SaveGameInstance->Poses.Remove(PoseName);

		// Set data on the savegame object.
		SaveGameInstance->Poses.Add(PoseName, Pose);

		// Save the data immediately.
		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("UltraleapSaveSlot"), 0))
		{
			// Save succeeded.
		}
	}
}

FPoseSnapshot ULeapPoseComponent::GetPose(FString PoseName)
{

	if (ULeapSaveGame* LoadedGame = Cast<ULeapSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("UltraleapSaveSlot"), 0)))
	{
		if (LoadedGame->Poses.Contains(PoseName))
		{
			LoadedGame->Poses[PoseName];
		}
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		// UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *LoadedGame->PlayerName);
	}


	return FPoseSnapshot();
}


