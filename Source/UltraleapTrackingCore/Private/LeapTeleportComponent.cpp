/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "LeapTeleportComponent.h"
#include "LeapUtility.h"
#include "LeapVisualizer.h"
#include "LeapHandActor.h"

// Sets default values for this component's properties
ULeapTeleportComponent::ULeapTeleportComponent() 
	: LocalTeleportLaunchSpeed(1000)
	, bValidTeleportationLocation(false)
	, bTeleportTraceActive(false)
	, bTeleportOnce(true)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	LeapTeleportTraceNS = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,
		TEXT("NiagaraSystem'/UltraleapTracking/InteractionEngine/VFX/Leap_NS_TeleportTrace.Leap_NS_TeleportTrace'")));
	if (LeapTeleportTraceNS == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapTeleportTraceNS is nullptr in ULeapTeleportComponent()"));
	}
}


// Called when the game starts
void ULeapTeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	WorldContextObject = GetWorld();

	if (!WorldContextObject)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("WorldContextObject is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}
	Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("Owner is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}

	LeapSubsystem = ULeapSubsystem::Get();
	if (LeapSubsystem == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapSubsystem is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}

	LeapSubsystem->OnLeapGrabActionNative.AddUObject(this, &ULeapTeleportComponent::OnLeapGrabAction);
	LeapSubsystem->OnLeapReleaseNative.AddUObject(this, &ULeapTeleportComponent::OnLeapRelease);

	NavSys = UNavigationSystemV1::GetNavigationSystem(WorldContextObject);
	if (!NavSys)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("NavSys is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}

	if (!CameraComponent)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("CameraComponent is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}
	
}

void ULeapTeleportComponent::StartTeleportTrace()
{
	bTeleportTraceActive = true;

	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;

	TeleportTraceNSComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, LeapTeleportTraceNS, Location, Rotation);

	if (!TeleportTraceNSComponent)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("TeleportTraceNSComponent is nullptr in ULeapTeleportComponent::StartTeleportTrace()"));
		return;
	}

	TeleportTraceNSComponent->SetVisibility(true);

	UClass* VisualizerClass = ALeapVisualizer::StaticClass();
	FVector Loc = FVector::ZeroVector;
	FRotator Rot = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams = FActorSpawnParameters();
	TeleportVisualizerReference = GetWorld()->SpawnActor(VisualizerClass, &Loc, &Rot, SpawnParams);

	if (!TeleportVisualizerReference)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("TeleportVisualizerReference is nullptr in ULeapTeleportComponent::BeginPlay()"));
		return;
	}

	TeleportVisualizerReference->AttachToActor(Owner, FAttachmentTransformRules::KeepWorldTransform);
}

bool ULeapTeleportComponent::IsValidTeleportLocation(FHitResult OutHit, FNavLocation &OutLocation)
{
	bool Valid = NavSys->ProjectPointToNavigation(OutHit.Location, OutLocation); 
	return (Valid && OutHit.bBlockingHit);
}

void ULeapTeleportComponent::TeleportTrace(const FVector Location, const FVector Direction)
{
	FHitResult OutHit;
	TArray<FVector> OutPathPositions;
	FVector OutLastTraceDestination;
	FVector StartPos = Location; 
	FVector LaunchVelocity = Direction * LocalTeleportLaunchSpeed;
	float ProjectileRadius = 3.6f;
	TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
	TEnumAsByte<EObjectTypeQuery> ObjectType{UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic)};
	ObjectTypes.Add(ObjectType);
	const TArray<AActor*> ActorsToIgnore;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	float DrawDebugTime = 0.0f;

	UGameplayStatics::Blueprint_PredictProjectilePath_ByObjectType(WorldContextObject, OutHit, OutPathPositions,
		OutLastTraceDestination, StartPos, LaunchVelocity, true, ProjectileRadius, ObjectTypes, false, ActorsToIgnore,
		DrawDebugType,
		DrawDebugTime);

	OutPathPositions.Insert(StartPos, 0);

	FNavLocation OutLocation;
	bValidTeleportationLocation = IsValidTeleportLocation(OutHit, OutLocation);

	if (bValidTeleportationLocation)
	{
		float LocalNavMeshCellHeight = 8.0f;
		if (USceneComponent* Component = TeleportVisualizerReference->GetRootComponent())
		{
			Component->SetVisibility(bValidTeleportationLocation);
		}
		ProjectedTeleportLocation = FVector(OutLocation.Location.X, OutLocation.Location.Y, OutLocation.Location.Z - LocalNavMeshCellHeight);
	}

	TeleportVisualizerReference->SetActorLocation(ProjectedTeleportLocation);
	const FName OverrideName = FName(TEXT("User.PointArray"));
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TeleportTraceNSComponent, OverrideName, OutPathPositions);
}

void ULeapTeleportComponent::TryTeleport()
{
	if (!bValidTeleportationLocation)
	{
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("not bValidTeleportationLocation in ULeapTeleportComponent::TryTeleport"));
		return;
	}
	bValidTeleportationLocation = false;

	if (!CameraComponent)
	{
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("nullptr CameraComponent in ULeapTeleportComponent::TryTeleport"));
		return;
	}

	FVector Location = FVector(CameraComponent->GetRelativeLocation().X, CameraComponent->GetRelativeLocation().Y, 0.0f);
	FRotator Rotation = FRotator(Owner->GetActorRotation());
	Location = Rotation.RotateVector(Location);
	Location = ProjectedTeleportLocation - Location;
	Owner->TeleportTo(Location, Rotation);

}

void ULeapTeleportComponent::EndTeleportTrace()
{
	bTeleportTraceActive = false;
	if (TeleportTraceNSComponent)
	{
		TeleportTraceNSComponent->DestroyComponent();
	}
	if (TeleportVisualizerReference)
	{
		TeleportVisualizerReference->Destroy();
	}
}


void ULeapTeleportComponent::OnLeapGrabAction(FVector Location, FVector ForwardVec)
{
	if (bTeleportOnce)
	{
		StartTeleportTrace();
		bTeleportOnce = false;
	}
	TeleportTrace(Location, ForwardVec);
}

void ULeapTeleportComponent::OnLeapRelease(
	AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (!ReleasedActor)
	{
		return;
	}
	if (!Cast<ALeapHandActor>(ReleasedActor))
	{
		return;
	}
	if (!bTeleportTraceActive)
	{	
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("bTeleportTraceActive is false in OnLeapRelease"));
		return;
	}
	bTeleportOnce = true;
	EndTeleportTrace();
	TryTeleport();
}
