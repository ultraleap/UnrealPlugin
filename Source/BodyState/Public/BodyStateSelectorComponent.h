// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BodyStateSkeleton.h"
#include "BodyStateEnums.h"
#include "BodyStateSelectorComponent.generated.h"

UENUM(BlueprintType, Category = "BS Enums")
enum class EBSSkeletonType : uint8
{
	BSSkeletonType_Auto,
	BSSkeletonType_Local
};

//Disable this component in bp until v3.1 when the multi-player selection and replication functionality gets implemented
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent)) 
class BODYSTATE_API UBodyStateSelectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBodyStateSelectorComponent();

	UPROPERTY(BlueprintAssignable, Category = "BS Selector")
	FBodyStateSkeletonSignature OnSkeletonChanged;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BS Selector")
	UBodyStateSkeleton* Skeleton;

	/** Force local, force network, or auto (default)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Selector")
	EBSSkeletonType SkeletonType;

	/** Only relevent for auto and Device, use non-zero to override*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Selector")
	int32 DeviceId;

	/** Determine if this is an owner or not*/
	UPROPERTY(BlueprintReadOnly, Category = "BS Selector")
	bool bIsLocallyControlled;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, Category = "BS Selector")
	bool bReplicatesSkeleton;

	UPROPERTY(ReplicatedUsing = OnRep_Flag)
	FNamedSkeletonData ReplicatedStruct;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	APawn* OwningPawn;

	//Replication
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerUpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

	//add: Unreliable
	UFUNCTION(Unreliable, Client)
	void Multi_UpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

	void SyncSkeletonAndAuthority();

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

};
