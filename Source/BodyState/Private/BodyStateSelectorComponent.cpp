// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BodyStateSelectorComponent.h"
#include "BodyStateBPLibrary.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "UnrealNetwork.h"

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
	bNetAddressable = true;
	OwningPawn = nullptr;
	bIsLocallyControlled = true;
	NetMode = ENetMode::NM_Standalone;
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
	OwningPawn = Cast<APawn>((AActor*)GetOwner()->GetNetOwner());

	if (!OwningPawn)
	{
		bReplicates = false;
		Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		return;
	}

	NetMode = OwningPawn->GetNetMode();

	//Sync authority
	UE_LOG(LogTemp, Log, TEXT("SyncSkeletonAndAuthorityRole: %d"), (int32)OwningPawn->Role);
	
	bIsLocallyControlled = OwningPawn->IsLocallyControlled(); //Owner->Role == ENetRole::ROLE_Authority || Owner->Role == ENetRole::ROLE_AutonomousProxy;

	//Local only, won't ever replicate
	if (SkeletonType == EBSSkeletonType::BSSkeletonType_Local || NetMode == NM_Standalone)
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
		else{
			Skeleton = nullptr;	//clear old
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
	//SyncSkeletonAndAuthority();
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

void UBodyStateSelectorComponent::ServerUpdateMinimalData_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	UE_LOG(LogTemp, Log, TEXT("Notify: (A%d), %d, %d, %s %s"), (int32)OwningPawn->Role, bIsLocallyControlled, bReplicatesSkeleton, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));

	MinimalSkeletalData = Skeleton->GetMinimalNamedSkeletonData();
}


void UBodyStateSelectorComponent::OnRep_NamedSkeletonData()
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_NamedSkeletonData: Skeleton not ready, skipping update"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("receiving data to: %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *(UKismetSystemLibrary::GetDisplayName(GetOwner())));

	if (bIsLocallyControlled)
	{
		UE_LOG(LogTemp, Log, TEXT("ignored"));
		//We should not update ourselves
		return;
	}
	Skeleton->SetFromNamedSkeletonData(MinimalSkeletalData);
	Skeleton->Name = TEXT("ActivelyNetworked");

	//Temp: Reset thumb so we can see the data is different from regularly fetched data
	FBodyStateBoneData Bone;
	Bone.Alpha = 1.0f;

	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R);

	UE_LOG(LogTemp, Log, TEXT("data set"));
}


bool UBodyStateSelectorComponent::ServerUpdateMinimalData_Validate(const FNamedSkeletonData InBodyStateSkeleton)
{
	return true;
}

void UBodyStateSelectorComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBodyStateSelectorComponent, MinimalSkeletalData);
}

void UBodyStateSelectorComponent::Multi_UpdateBodyState_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("Multi_UpdateBodyState_Implementation: Skeleton not ready, skipping update"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("receiving data to: %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *(UKismetSystemLibrary::GetDisplayName(GetOwner())));

	if (bIsLocallyControlled)
	{
		UE_LOG(LogTemp, Log, TEXT("ignored"));
		//We should not update ourselves
		return;
	}
	Skeleton->SetFromNamedSkeletonData(InBodyStateSkeleton);
	Skeleton->Name = TEXT("ActivelyNetworked");

	//Temp: Reset thumb so we can see the data is different from regularly fetched data
	FBodyStateBoneData Bone;
	Bone.Alpha = 1.0f;

	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R);
	Skeleton->SetDataForBone(Bone, EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R);

	UE_LOG(LogTemp, Log, TEXT("data set"));
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
	//UE_LOG(LogTemp, Log, TEXT("Evaluating tick data from: (A%d), %d, %d, %s %s"), (int32)OwningPawn->Role, bIsLocallyControlled, bReplicatesSkeleton, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));
	
	if (bReplicatesSkeleton && bIsLocallyControlled && Skeleton)
	{
		

		//function method

		//update server
		if (OwningPawn->Role < ROLE_Authority)
		{
			UE_LOG(LogTemp, Log, TEXT("Set data from: (A%d), %d, %d, %s %s"), (int32)OwningPawn->Role, bIsLocallyControlled, bReplicatesSkeleton, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));
			ServerUpdateMinimalData(Skeleton->GetMinimalNamedSkeletonData());
			

			//UE_LOG(LogTemp, Log, TEXT("Sending data from: %d, %s %s"), bIsLocallyControlled, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));
			//ServerUpdateBodyState_Implementation(MinimalSkeletalData);
		}
		//multicast to everyone
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Server Set data from: (A%d), %d, %d, %s %s"), (int32)OwningPawn->Role, bIsLocallyControlled, bReplicatesSkeleton, *Skeleton->Name, *UKismetSystemLibrary::GetDisplayName(GetOwner()));

			MinimalSkeletalData = Skeleton->GetMinimalNamedSkeletonData();
			//Multi_UpdateBodyState(MinimalSkeletalData);
		}

	}
}