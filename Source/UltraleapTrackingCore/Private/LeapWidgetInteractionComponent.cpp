// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapWidgetInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "LeapUtility.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) 
	: Super(ObjectInitializer)
	, CursorDistanceFromHand(30)
	, StaticMesh(nullptr)
	, MaterialBase(nullptr)
	, CursorSize(0.03) 
	, HandType(EHandType::LEAP_HAND_LEFT)
	, WidgetInteraction(EUIType::FAR)
	, LeapSubsystem(nullptr)
	, InterpolationDelta(0.01)
	, InterpolationSpeed(10)
	, LeapPawn(nullptr)
	, PointerActor(nullptr)
	, World(nullptr)
	, LeapDynMaterial(nullptr)
	, PlayerCameraManager(nullptr)
	, bIsPinched(false)
	, bHandTouchWidget(false)
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
	if (StaticMesh==nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("StaticMesh is nullptr in CreatStaticMeshForCursor()"));
		return;
	}
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("/UltraleapTracking/Explore/Diamond_Mat.Diamond_Mat"));
	
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetWorldScale3D(CursorSize*FVector(1, 1, 1));
	}
}

bool ULeapWidgetInteractionComponent::NearlyEqualVectors(FVector A, FVector B, float ErrorTolerance)
{
	FVector Diff = A - B;
	return FMath::IsNearlyEqual(Diff.Size(), SMALL_NUMBER, ErrorTolerance);
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
		// The cursor position is the addition of the Pawn pose and the hand pose
		FVector HandLocation = FVector();
		if (WidgetInteraction == EUIType::NEAR)
		{
			HandLocation = TmpHand.Index.Distal.NextJoint;
		}
		else
		{
			HandLocation = TmpHand.Index.Metacarpal.PrevJoint;
		}

		FVector PawnLocation = LeapPawn->GetActorLocation();
		FVector CursorLocation = HandLocation + PawnLocation;
		FTransform TargetTrans = FTransform();
		TargetTrans.SetLocation(CursorLocation);

		// The direction is the line between the hmd and the hand, CursorDistanceFromHand is used to add an offset in the palm direction 
		FVector Direction = CursorLocation + (CursorDistanceFromHand * TmpHand.Palm.Direction) - PlayerCameraManager->GetTransform().GetLocation();
		Direction.Normalize();
		TargetTrans.SetRotation(Direction.Rotation().Quaternion());

		//Interp is needed to reduce the jitter
		FTransform NewTransform = UKismetMathLibrary::TInterpTo(GetComponentTransform(), TargetTrans, InterpolationDelta, InterpolationSpeed);
		// This will set this component's transform with the more stable interp
		SetWorldTransform(NewTransform);
		// Set the sphere location in the widget using the hit result inherited from the parent class
		StaticMesh->SetWorldLocation(LastHitResult.ImpactPoint);

		if (WidgetInteraction == EUIType::NEAR)
		{
			if (NearlyEqualVectors(CursorLocation, LastHitResult.ImpactPoint, 4E+0F))
			{
				NearClickLeftMouse();
			}
			else
			{
				NearReleaseLeftMouse();
			}
		}
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("nullptr in DrawLeapCircles"));
	}
}

void ULeapWidgetInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();
	if (GEngine == nullptr || World == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("GEngine or World is nullptr in BeginPlay"));
		return;
	}
	LeapSubsystem = GEngine->GetEngineSubsystem<UULeapSubsystem>();
	if (LeapSubsystem==nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapSubsystem is nullptr in BeginPlay"));
		return;
	}
	LeapPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (LeapPawn == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapPawn is nullptr in BeginPlay"));
		return;
	}
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	if (PlayerCameraManager == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("PlayerCameraManager is nullptr in BeginPlay"));
		return;
	}

	// Subscribe events from leap, for pinch, unpinch and get the tracking data
	if (WidgetInteraction != EUIType::NEAR)
	{
		LeapSubsystem->OnLeapPinchMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapPinch);
		LeapSubsystem->OnLeapUnPinchMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapUnPinch);
	}
	// Will need to get tracking data regardless of the interaction type (near or far)
	LeapSubsystem->OnLeapFrameMulti.AddDynamic(this, &ULeapWidgetInteractionComponent::OnLeapTrackingData);
	
	if (MaterialBase != nullptr)
	{
		LeapDynMaterial = UMaterialInstanceDynamic::Create(MaterialBase, NULL);
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("MaterialBase is nullptr in BeginPlay"));
		return;
	}
	
	if (LeapDynMaterial != nullptr && StaticMesh != nullptr)
	{
		StaticMesh->SetMaterial(0, LeapDynMaterial);
		FVector Scale = CursorSize * FVector(1, 1, 1);
		StaticMesh->SetWorldScale3D(Scale);
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapDynMaterial or  StaticMesh is nullptr in BeginPlay"));
		return;
	}
}

void ULeapWidgetInteractionComponent::OnLeapPinch(const FLeapHandData& HandData)
{
	if (HandData.HandType == HandType && StaticMesh)
	{
		// Onpinch scale the cursor by 1/2
		FVector Scale = CursorSize * FVector(1, 1, 1);
		Scale = Scale / 2;
		StaticMesh->SetWorldScale3D(Scale);
		// Press the LeftMouseButton
		PressPointerKey(EKeys::LeftMouseButton);
		bIsPinched = true;
	}
}

void ULeapWidgetInteractionComponent::OnLeapUnPinch(const FLeapHandData& HandData)
{
	if (HandData.HandType == HandType && StaticMesh && bIsPinched)
	{
		// OnUnpinch scale the cursor by 2
		FVector Scale = StaticMesh->GetComponentScale();
		if (FMath::IsNearlyEqual(Scale.X, (CursorSize / 2), 1.0E-2F))
		{
			Scale = Scale * 2;
			StaticMesh->SetWorldScale3D(Scale);
		}
		//Release the LeftMouseButton
		ReleasePointerKey(EKeys::LeftMouseButton);
		bIsPinched = false;
	}	
}

void ULeapWidgetInteractionComponent::NearClickLeftMouse()
{
	if (!bHandTouchWidget)
	{
		FVector Scale = CursorSize * FVector(1, 1, 1);
		Scale = Scale / 2;
		StaticMesh->SetWorldScale3D(Scale);
		// Press the LeftMouseButton
		PressPointerKey(EKeys::LeftMouseButton);
		bHandTouchWidget = true;

		UE_LOG(UltraleapTrackingLog, Error, TEXT("<<<<<<<< Mouse clicked >>>>>>>>"));
	}
}

void ULeapWidgetInteractionComponent::NearReleaseLeftMouse()
{
	if (bHandTouchWidget)
	{
		FVector Scale = StaticMesh->GetComponentScale();
		if (FMath::IsNearlyEqual(Scale.X, (CursorSize / 2), 1.0E-2F))
		{
			Scale = Scale * 2;
			StaticMesh->SetWorldScale3D(Scale);
		}
		// Release the LeftMouseButton
		ReleasePointerKey(EKeys::LeftMouseButton);
		bHandTouchWidget = false;

		UE_LOG(UltraleapTrackingLog, Error, TEXT("######## Mouse Released #########"));
	}
}

void ULeapWidgetInteractionComponent::SpawnStaticMeshActor(const FVector& InLocation)
{
	if (World == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("World is nullptr in SpawnStaticMeshActor"));
		return;
	}
	PointerActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	if (PointerActor == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("PointerActor is nullptr in SpawnStaticMeshActor"));
		return;
	}
	PointerActor->SetMobility(EComponentMobility::Movable);
	PointerActor->SetActorLocation(InLocation);
	UStaticMeshComponent* MeshComponent = PointerActor->GetStaticMeshComponent();
	if (MeshComponent && StaticMesh)
	{
		MeshComponent->SetStaticMesh(StaticMesh->GetStaticMesh());
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("MeshComponent or StaticMesh is nullptr in SpawnStaticMeshActor"));
	}
}

void ULeapWidgetInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Clean up the subsystem events 
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapFrameMulti.Clear();
		if (WidgetInteraction != EUIType::NEAR)
		{
			LeapSubsystem->OnLeapPinchMulti.Clear();
			LeapSubsystem->OnLeapUnPinchMulti.Clear();
		}
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
