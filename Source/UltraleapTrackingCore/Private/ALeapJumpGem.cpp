/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "ALeapJumpGem.h"
#include "LeapUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AALeapJumpGem::AALeapJumpGem()
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
	UMaterial* MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("/UltraleapTracking/Explore/Diamond_Mat.Diamond_Mat"));
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
void AALeapJumpGem::BeginPlay()
{
	Super::BeginPlay();

	LeapSubsystem = GEngine->GetEngineSubsystem<UULeapSubsystem>();
	if (LeapSubsystem!=nullptr)
	{
		LeapSubsystem->OnLeapGrabNative.BindUObject(this, &AALeapJumpGem::OnGrabbed);
		LeapSubsystem->OnLeapReleaseNative.BindUObject(this, &AALeapJumpGem::OnReleased);
		LeapSubsystem->OnLeapTrackingDatanative.BindUObject(this, &AALeapJumpGem::OnLeapTrackingData);
	}

}

// Called every frame
void AALeapJumpGem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AALeapJumpGem::OnGrabbed(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight)
{
	if (GrabbedActor != nullptr && this == GrabbedActor)
	{
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AALeapJumpGem::RepeatingAction, 0.04f, true, .0f);
	}
}

void AALeapJumpGem::OnReleased(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor != nullptr && this == ReleasedActor && HandLeft != nullptr)
	{
		ReleasedActor->AttachToComponent(HandLeft, FAttachmentTransformRules::SnapToTargetNotIncludingScale, BoneName);
		ReleasedActor->SetActorRelativeLocation(FVector(-2,0,7));
		ReleasedActor->SetActorRelativeRotation(FRotator(0, 90, 0));

		if (GetWorldTimerManager().IsTimerActive(TimerHandle))
		{
			GetWorldTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void AALeapJumpGem::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;
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

bool AALeapJumpGem::IsLeftHandFacingCamera(FLeapHandData Hand)
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (PlayerCameraManager!=nullptr)
	{
		FVector CamLocation = PlayerCameraManager->GetActorRightVector();
		FRotator PalmRot = Hand.Palm.Orientation;
		FVector PalmRotForward = UKismetMathLibrary::GetRightVector(PalmRot);
		float DotProd = FVector::DotProduct(CamLocation, PalmRotForward);
		if (UKismetMathLibrary::InRange_FloatFloat(DotProd, -1.f, -0.4f))
		{
			this->SetActorHiddenInGame(false);
			return true;
		}
	}
	this->SetActorHiddenInGame(true);
	return false;
}

void AALeapJumpGem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapGrabNative.Unbind();
		LeapSubsystem->OnLeapReleaseNative.Unbind();
		LeapSubsystem->OnLeapTrackingDatanative.Unbind();
	}

	if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
	}
	
}

void AALeapJumpGem::RepeatingAction()
{
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->GrabActionCall(GetActorLocation(), GetActorForwardVector());
	}
}
