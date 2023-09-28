// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapWidgetInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "LeapUtility.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent(
	const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) 
	: Super(ObjectInitializer)
	, CursorDistanceFromHand(30)
	, StaticMesh(nullptr)
	, MaterialBase(nullptr)
	, CursorSize(0.03) 
	, HandType(EHandType::LEAP_HAND_LEFT)
	, LeapSubsystem(nullptr)
	, LeapPawn(nullptr)
	, PointerActor(nullptr)
	, World(nullptr)
	, LeapDynMaterial(nullptr)
	, PlayerCameraManager(nullptr)
	, bIsPinched(false)
{
	CreatStaticMeshForCursor();
}

ULeapWidgetInteractionComponent::~ULeapWidgetInteractionComponent()
{

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
		StaticMesh->SetWorldScale3D(CursorSize*FVector(1, 1, 1));
	}
}

void ULeapWidgetInteractionComponent::DrawLeapCursor(FLeapHandData& Hand)
{
	FLeapHandData TmpHand = Hand;
	if (TmpHand.HandType != HandType)
	{
		return;
	}

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
		LeapSubsystem->OnLeapPinchMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapPinch);
		LeapSubsystem->OnLeapUnPinchMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapUnPinch);
		LeapSubsystem->OnLeapFrameMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapTrackingData);
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
	
	if (LeapDynMaterial != nullptr && StaticMesh != nullptr)
	{
		StaticMesh->SetMaterial(0, LeapDynMaterial);

		FVector Scale = CursorSize * FVector(1, 1, 1);
		StaticMesh->SetWorldScale3D(Scale);
	}
}

void ULeapWidgetInteractionComponent::OnLeapPinch(const FLeapHandData& HandData)
{

	UE_LOG(UltraleapTrackingLog, Error, TEXT("HandType %i"), (uint8)HandType);

	if (HandData.HandType == HandType && StaticMesh)
	{
		FVector Scale = CursorSize * FVector(1, 1, 1);
		Scale = Scale / 2;
		StaticMesh->SetWorldScale3D(Scale);
		PressPointerKey(EKeys::LeftMouseButton);

		bIsPinched = true;
	}
}

void ULeapWidgetInteractionComponent::OnLeapUnPinch(const FLeapHandData& HandData)
{
	if (HandData.HandType == HandType && StaticMesh && bIsPinched)
	{
		FVector Scale = StaticMesh->GetComponentScale();
		if (FMath::IsNearlyEqual(Scale.X, (CursorSize / 2), 1.0E-2F))
		{
			Scale = Scale * 2;
			StaticMesh->SetWorldScale3D(Scale);
		}
		ReleasePointerKey(EKeys::LeftMouseButton);

		bIsPinched = false;
	}	
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
		LeapSubsystem->OnLeapPinchMulti.Clear();
		LeapSubsystem->OnLeapUnPinchMulti.Clear();
		LeapSubsystem->OnLeapFrameMulti.Clear();
	}
}

void ULeapWidgetInteractionComponent::InitializeComponent()
{

}

void ULeapWidgetInteractionComponent::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;
	for (int32 i = 0; i < Hands.Num(); ++i)
	{
		DrawLeapCursor(Hands[i]);
		switch (Hands[i].HandType)
		{
			case EHandType::LEAP_HAND_LEFT:
				break;
			case EHandType::LEAP_HAND_RIGHT:
				break;
			default:
				break;
		}
	}
}
