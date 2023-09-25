// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapWidgetInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent()
{
	CreatStaticMeshForCursor();
}

void ULeapWidgetInteractionComponent::CreatStaticMeshForCursor()
{

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(
		TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorMesh"));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UMaterialInstanceDynamic* LeapDynMaterial = nullptr;
	UMaterial* MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("/UltraleapTracking/Explore/Diamond_Mat.Diamond_Mat"));
	if (MaterialBase != nullptr)
	{
		LeapDynMaterial = UMaterialInstanceDynamic::Create(MaterialBase, NULL);
	}
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeScale3D(FVector(0.03f, 0.03f, 0.03f));
		if (LeapDynMaterial != nullptr)
		{
			StaticMesh->SetMaterial(0, LeapDynMaterial);
		}
	}
}

void ULeapWidgetInteractionComponent::DrawLeapCircles(FLeapHandData& Hand)
{
	FLeapHandData TmpHand = Hand;
	//TmpHand.Palm.Orientation.Pitch = Hand.Palm.Orientation.Pitch - 60;
	if (StaticMesh != nullptr && LeapPawn!=nullptr)
	{
		FVector HandLocation = TmpHand.Palm.Position;
		FVector PawnLocation = LeapPawn->GetActorLocation();
		FVector CursorLocation = HandLocation + PawnLocation;
		FTransform TargetTrans = FTransform();
		TargetTrans.SetLocation(CursorLocation);
		TargetTrans.SetRotation(TmpHand.Palm.Orientation.Quaternion());

		FTransform NewTransform = UKismetMathLibrary::TInterpTo(GetComponentTransform(), TargetTrans, 0.01, 10);
		SetWorldTransform(NewTransform);
		StaticMesh->SetWorldLocation(LastHitResult.ImpactPoint);
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
	}

	World = GetWorld();
	if (World)
	{
		LeapPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	}

	SetWorldRotation(FRotator(0,30,0));
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
}

void ULeapWidgetInteractionComponent::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;
	for (int32 i = 0; i < Hands.Num(); ++i)
	{
		switch (Hands[i].HandType)
		{
			case EHandType::LEAP_HAND_LEFT:
				DrawLeapCircles(Hands[i]);
				//IsLeftHandFacingCamera(Hands[i]);
				break;
			case EHandType::LEAP_HAND_RIGHT:
				break;
			default:
				break;
		}
	}
}
