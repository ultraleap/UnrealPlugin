// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BodyStateSelectorComponent.h"
#include "BodyStateBPLibrary.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"


// Sets default values for this component's properties
UBodyStateSelectorComponent::UBodyStateSelectorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Skeleton = nullptr;
	SkeletonType = EBSSkeletonType::BSSkeletonType_Auto;
	DeviceId = 0;
	bReplicates = false;
}


// Called when the game starts
void UBodyStateSelectorComponent::BeginPlay()
{
	Super::BeginPlay();

	SyncSkeletonAndAuthority();
}

void UBodyStateSelectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Skeleton = nullptr;
}

void UBodyStateSelectorComponent::SyncSkeletonAndAuthority()
{
	//Who are we owned by? cache authority and replication status
	bReplicatesSkeleton = false;
	bHasAuthority = true;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		return;
	}

	//Sync authority
	bHasAuthority = Owner->HasAuthority();

	//Local only, won't ever replicate
	if (SkeletonType == EBSSkeletonType::BSSkeletonType_Local)
	{
		Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
	}
	//can be will be auto based on network and ownership
	else
	{
		//owner? we will be replicating outward
		if (bHasAuthority)
		{
			bReplicatesSkeleton = true;
			Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		}
		//I'll be receiving skeleton data
		else if(!Skeleton)
		{
			Skeleton = NewObject<UBodyStateSkeleton>(this);
		}
	}
}

void UBodyStateSelectorComponent::InitializeComponent()
{
	SyncSkeletonAndAuthority();
}

void UBodyStateSelectorComponent::UninitializeComponent()
{
	Skeleton = nullptr;
}

void UBodyStateSelectorComponent::ServerUpdateBodyState_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	// Multi cast to everybody
	Multi_UpdateBodyState(InBodyStateSkeleton);
}

void UBodyStateSelectorComponent::Multi_UpdateBodyState_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("Multi_UpdateBodyState_Implementation: Skeleton not ready, skipping update"));
		return;
	}
	Skeleton->SetFromNamedSkeletonData(InBodyStateSkeleton);
	Skeleton->Name = TEXT("Network");
}

bool UBodyStateSelectorComponent::ServerUpdateBodyState_Validate(const FNamedSkeletonData InBodyStateSkeleton)
{
	return true;
}

// Called every frame
void UBodyStateSelectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Replicate to other clients
	if (bReplicatesSkeleton && Skeleton)
	{
		ServerUpdateBodyState_Implementation(Skeleton->GetMinimalNamedSkeletonData());
	}
}