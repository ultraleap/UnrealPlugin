// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BodyStateSelectorComponent.h"
#include "BodyStateBPLibrary.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"

// Sets default values for this component's properties
UBodyStateSelectorComponent::UBodyStateSelectorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Skeleton = nullptr;
	SkeletonType = EBSSkeletonType::BSSkeletonType_Auto;
	DeviceId = 0;
	bReplicates = true;
	OwningPawn = nullptr;
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
	bIsLocallyControlled = true;
	UBodyStateSkeleton* OldSkeleton = Skeleton;

	OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		return;
	}

	//Sync authority
	UE_LOG(LogTemp, Log, TEXT("SyncSkeletonAndAuthorityRole: %d"), (int32)OwningPawn->Role);
	
	bIsLocallyControlled = OwningPawn->IsLocallyControlled(); //Owner->Role == ENetRole::ROLE_Authority || Owner->Role == ENetRole::ROLE_AutonomousProxy;

	//Local only, won't ever replicate
	if (SkeletonType == EBSSkeletonType::BSSkeletonType_Local)
	{
		Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
	}
	//can be will be auto based on network and ownership
	else
	{
		//owner? we will be replicating outward
		if (bIsLocallyControlled)
		{
			bReplicatesSkeleton = true;
			Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		}
		//I'll be receiving skeleton data
		//else if(!Skeleton)
		else{
			Skeleton = NewObject<UBodyStateSkeleton>(this, TEXT("NRBodyStateSkeleton"));
			Skeleton->Name = TEXT("NetworkReplicated");
			Skeleton->SkeletonId = -1;
		}
	}

	bReplicates = bReplicatesSkeleton;

	if (Skeleton != OldSkeleton)
	{
		OnSkeletonChanged.Broadcast(Skeleton);
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
	UE_LOG(LogTemp, Log, TEXT("Server : %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));
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
	if (bIsLocallyControlled)
	{
		//We should not update ourselves
		return;
	}
	Skeleton->SetFromNamedSkeletonData(InBodyStateSkeleton);
	Skeleton->Name = TEXT("ActivelyNetworked");

	UE_LOG(LogTemp, Log, TEXT("receiving data to: %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *(UKismetSystemLibrary::GetDisplayName(GetOwner()) ));

	//Temp: Reset thumb so we know the data is different from regularly fetched data
	FBodyStateBoneData Bone;
	Bone.Alpha = 1.0f;

	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R);
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
	if (bReplicatesSkeleton && bIsLocallyControlled && Skeleton)
	{
		UE_LOG(LogTemp, Log, TEXT("Sending data from: %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));
		const FNamedSkeletonData MinimalSkeleton = Skeleton->GetMinimalNamedSkeletonData();

		//update server
		if (OwningPawn->Role < ROLE_Authority)
		{
			ServerUpdateBodyState_Implementation(MinimalSkeleton);
		}
		//multicast to everyone
		else
		{
			Multi_UpdateBodyState(MinimalSkeleton);
		}
	}
}