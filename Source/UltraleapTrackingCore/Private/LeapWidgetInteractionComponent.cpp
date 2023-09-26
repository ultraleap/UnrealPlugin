// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapWidgetInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "LeapUtility.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent() 
	: CursorDistanceFromHand(30)
	, LeapPawn(nullptr)
	, PointerActor(nullptr)
	, World(nullptr)
	, StaticMesh(nullptr)
	, PlayerCameraManager(nullptr)
	, MaterialBase(nullptr)
	, LeapDynMaterial(nullptr)
{
	CreatStaticMeshForCursor();
}

void ULeapWidgetInteractionComponent::CreatStaticMeshForCursor()
{

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorMesh"));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("/UltraleapTracking/Explore/Diamond_Mat.Diamond_Mat"));
	
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeScale3D(FVector(0.03f, 0.03f, 0.03f));
	}
}

void ULeapWidgetInteractionComponent::DrawLeapCursor(FLeapHandData& Hand)
{
	FLeapHandData TmpHand = Hand;

	if (StaticMesh != nullptr && LeapPawn != nullptr && PlayerCameraManager != nullptr)
	{
		FVector HandLocation = TmpHand.Index.Proximal.PrevJoint;
		FVector PawnLocation = LeapPawn->GetActorLocation();
		FVector CursorLocation = HandLocation + PawnLocation;
		FTransform TargetTrans = FTransform();

		TargetTrans.SetLocation(CursorLocation);
		FVector Direction = CursorLocation + (CursorDistanceFromHand * TmpHand.Palm.Direction) - PlayerCameraManager->GetTransform().GetLocation();

		Direction.Normalize();

		TargetTrans.SetRotation(Direction.Rotation().Quaternion());

		FTransform NewTransform = UKismetMathLibrary::TInterpTo(GetComponentTransform(), TargetTrans, 0.01, 10);
		SetWorldTransform(NewTransform);
		

		StaticMesh->SetWorldLocation(LastHitResult.ImpactPoint);
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("nullptr in DrawLeapCircles"));
	}
}

void ULeapWidgetInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine != nullptr)
	{
		LeapSubsystem = GEngine->GetEngineSubsystem<UULeapSubsystem>();
	}

	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapTrackingDatanative.BindUObject(this, &ULeapWidgetInteractionComponent::OnLeapTrackingData);
		LeapSubsystem->OnLeapPinch.BindUObject(this, &ULeapWidgetInteractionComponent::OnLeapPinch);
		LeapSubsystem->OnLeapUnpinched.BindUObject(this, &ULeapWidgetInteractionComponent::OnLeapUnPinch);
	}

	World = GetWorld();
	if (World)
	{
		LeapPawn = UGameplayStatics::GetPlayerPawn(World, 0);
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	}

	if (MaterialBase != nullptr)
	{
		LeapDynMaterial = UMaterialInstanceDynamic::Create(MaterialBase, NULL);
	}
	
	if (LeapDynMaterial != nullptr)
	{
		StaticMesh->SetMaterial(0, LeapDynMaterial);
	}
	
}

void ULeapWidgetInteractionComponent::OnLeapPinch(const FLeapHandData& HandData)
{
	if (StaticMesh)
	{
		FVector Scale = StaticMesh->GetRelativeScale3D();
		Scale = Scale / 2;
		StaticMesh->SetRelativeScale3D(Scale);
	}
	PressPointerKey(EKeys::LeftMouseButton);
}

void ULeapWidgetInteractionComponent::OnLeapUnPinch(const FLeapHandData& HandData)
{
	if (StaticMesh)
	{
		FVector Scale = StaticMesh->GetRelativeScale3D();
		Scale = Scale * 2;
		StaticMesh->SetRelativeScale3D(Scale);
	}
	ReleasePointerKey(EKeys::LeftMouseButton);
}

void ULeapWidgetInteractionComponent::SpawnStaticMeshActor(const FVector& InLocation)
{
	if (World == nullptr)
	{
		return;
	}
	PointerActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	if (PointerActor == nullptr)
	{
		return;
	}
	PointerActor->SetMobility(EComponentMobility::Movable);
	PointerActor->SetActorLocation(InLocation);
	UStaticMeshComponent* MeshComponent = PointerActor->GetStaticMeshComponent();
	if (MeshComponent && StaticMesh)
	{
		MeshComponent->SetStaticMesh(StaticMesh->GetStaticMesh());
	}
}

void ULeapWidgetInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapTrackingDatanative.Unbind();
		LeapSubsystem->OnLeapPinch.Unbind();
		LeapSubsystem->OnLeapUnpinched.Unbind();
	}
}

void ULeapWidgetInteractionComponent::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;
	for (int32 i = 0; i < Hands.Num(); ++i)
	{
		switch (Hands[i].HandType)
		{
			case EHandType::LEAP_HAND_LEFT:
				DrawLeapCursor(Hands[i]);
				//IsLeftHandFacingCamera(Hands[i]);
				break;
			case EHandType::LEAP_HAND_RIGHT:
				break;
			default:
				break;
		}
	}
}
