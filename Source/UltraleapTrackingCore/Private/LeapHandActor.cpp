/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "LeapHandActor.h"
#include "LeapUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"

// Sets default values
ALeapHandActor::ALeapHandActor() 
	: GrabPoseOffset(FVector(20, 0, 0)), ReleasePoseOffset(FVector(-2,0,7)), bIsHandFacingCamera(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Definition for the Mesh that will serve as our visual representation.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Engine/VREditor/TransformGizmo/StartRotationHandleIndicator.StartRotationHandleIndicator'"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpGem"));
	if (StaticMesh!=nullptr)
	{
		StaticMesh->SetupAttachment(RootComponent);
	}
	UMaterialInstanceDynamic* LeapDynMaterial = nullptr;
	UMaterial* MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("/UltraleapTracking/InteractionEngine/Materials/IE2_Materials/Diamond_Mat.Diamond_Mat"));
	if (MaterialBase != nullptr)
	{
		LeapDynMaterial = UMaterialInstanceDynamic::Create(MaterialBase, NULL);
	}
    if (DefaultMesh.Succeeded())
    {
        StaticMesh->SetStaticMesh(DefaultMesh.Object);
        StaticMesh->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));

        if (LeapDynMaterial != nullptr)
		{
			StaticMesh->SetMaterial(0, LeapDynMaterial);
		}
    }
}

// Called when the game starts or when spawned
void ALeapHandActor::BeginPlay()
{
	Super::BeginPlay();
	if (GEngine!=nullptr)
	{
		LeapSubsystem = ULeapSubsystem::Get();
	}
	
	if (LeapSubsystem!=nullptr)
	{
		LeapSubsystem->OnLeapGrabNative.AddUObject(this, &ALeapHandActor::OnGrabbed);
		LeapSubsystem->OnLeapReleaseNative.AddUObject(this, &ALeapHandActor::OnReleased);
		LeapSubsystem->OnLeapFrameMulti.AddUObject(this, &ALeapHandActor::OnLeapTrackingData);
	}

	World = GetWorld();

	if (World == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("World is nullptr in ALeapHandActor::BeginPlay()"));
		return;
	}

}

// Called every frame
void ALeapHandActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALeapHandActor::OnGrabbed(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight)
{
	if (GrabbedActor != nullptr && this == GrabbedActor && World)
	{
		// Offset needed so teleportation trace will not collide with the hand
		GrabbedActor->SetActorRelativeLocation(GrabPoseOffset);
		// Trigger a timer when the actor is grabbed
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ALeapHandActor::RepeatingAction, World->GetDeltaSeconds(), true, .0f);
	}
}

void ALeapHandActor::OnReleased(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor != nullptr && this == ReleasedActor && HandLeft != nullptr)
	{
		// Attach the gem to the hand
		ReleasedActor->AttachToComponent(HandLeft, FAttachmentTransformRules::SnapToTargetNotIncludingScale, BoneName);
		// Set up the posistion and rotation of the gem relative to the palm
		ReleasedActor->SetActorRelativeLocation(ReleasePoseOffset);
		ReleasedActor->SetActorRelativeRotation(FRotator::ZeroRotator);

		// Stop the timer when the actor is released
		if (GetWorldTimerManager().IsTimerActive(TimerHandle))
		{
			GetWorldTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void ALeapHandActor::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	Hands = Frame.Hands;
	for (int32 i = 0; i < Hands.Num(); ++i)
	{
		switch (Hands[i].HandType)
		{
			case EHandType::LEAP_HAND_LEFT:
				IsLeftHandFacingCamera(Hands[i]);
				break;
			case EHandType::LEAP_HAND_RIGHT:
				break;
			default:
				break;
		}	
	}
}

bool ALeapHandActor::IsLeftHandFacingCamera(FLeapHandData Hand)
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (PlayerCameraManager!=nullptr)
	{
		FVector CamLocation = PlayerCameraManager->GetActorRightVector();
		FRotator PalmRot = Hand.Palm.Orientation;
		FVector PalmRotForward = UKismetMathLibrary::GetRightVector(PalmRot);
		float DotProd = FVector::DotProduct(CamLocation, PalmRotForward);
		// -1.f is when the two vectors are parallel, -0.4f just for the range 
		if (UKismetMathLibrary::InRange_FloatFloat(DotProd, -1.f, -0.4f))
		{
			
			SetActorHiddenInGame(false);
			StaticMesh->SetHiddenInGame(false, true);
			if (!bIsHandFacingCamera)
			{
				bIsHandFacingCamera = true;
				OnLeapHandFaceCamera.Broadcast(bIsHandFacingCamera);
			}

			return true;
		}
	}

	SetActorHiddenInGame(true);
	StaticMesh->SetHiddenInGame(true, true);

	if (bIsHandFacingCamera)
	{
		bIsHandFacingCamera = false;
		OnLeapHandFaceCamera.Broadcast(bIsHandFacingCamera);
	}

	return false;
}

void ALeapHandActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Unbind all the events 
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapPinchMulti.Clear();
		LeapSubsystem->OnLeapUnPinchMulti.Clear();
		LeapSubsystem->OnLeapFrameMulti.Clear();
	}

	if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
	}
	
}

void ALeapHandActor::RepeatingAction()
{
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->GrabActionCall(GetActorLocation(), GetActorForwardVector());		
	}
}
