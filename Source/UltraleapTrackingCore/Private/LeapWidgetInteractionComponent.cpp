/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "LeapWidgetInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "LeapUtility.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) 
	: Super(ObjectInitializer)
	
	
	, LeapHandType(EHandType::LEAP_HAND_LEFT)
	, WidgetInteraction(EUIType::FAR)
	, StaticMesh(nullptr)
	, MaterialBase(nullptr)
	, CursorSize(0.03) 
	, CursorDistanceFromHand(50)
	, InterpolationDelta(0.01)
	, InterpolationSpeed(10)
	, bAutoMode(false)
	, IndexDitanceFromUI(0.00f)
	, LeapSubsystem(nullptr)
	, LeapPawn(nullptr)
	, PointerActor(nullptr)
	, World(nullptr)
	, LeapDynMaterial(nullptr)
	, PlayerCameraManager(nullptr)
	, bIsPinched(false)
	, bHandTouchWidget(false)
	, bAutoModeTrigger(false)
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
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MaterialBase = LoadObject<UMaterial>(nullptr, TEXT("Material'/UltraleapTracking/InteractionEngine2/Materials/M_LaserPointer-Outer.M_LaserPointer-Outer'"));
	
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetWorldScale3D(CursorSize*FVector(1, 1, 1));
	}
}

void ULeapWidgetInteractionComponent::HandleAutoMode(float Dist)
{
	if (bAutoMode)
	{
		if (Dist < 40 && WidgetInteraction == EUIType::FAR && !bAutoModeTrigger)
		{
			WidgetInteraction = EUIType::NEAR;
			bAutoModeTrigger = true;
			StaticMesh->SetHiddenInGame(true);
		}
		else if (Dist > 55 && WidgetInteraction == EUIType::NEAR && bAutoModeTrigger)
		{
			WidgetInteraction = EUIType::FAR;
			bAutoModeTrigger = false;
			StaticMesh->SetHiddenInGame(false);
		}
	}
}

void ULeapWidgetInteractionComponent::DrawLeapCursor(FLeapHandData& Hand)
{
	FLeapHandData TmpHand = Hand;
	if (TmpHand.HandType != LeapHandType)
	{
		return;
	}

	if (StaticMesh != nullptr && LeapPawn != nullptr && PlayerCameraManager != nullptr)
	{
		// The cursor position is the addition of the Pawn pose and the hand pose
		FVector HandLocation = FVector();
		if (WidgetInteraction == EUIType::NEAR)
		{
			HandLocation = TmpHand.Index.Intermediate.PrevJoint;
		}
		else
		{
			HandLocation = TmpHand.Middle.Metacarpal.PrevJoint;
		}

		FVector CursorLocation = HandLocation;
		FTransform TargetTrans = FTransform();
		TargetTrans.SetLocation(CursorLocation);

		FVector Direction = FVector();

		// Distal index used for Near interaction direction, Metacarpal for far ones
		if (WidgetInteraction == EUIType::NEAR)
		{
			Direction = TmpHand.Index.Distal.NextJoint - TmpHand.Index.Distal.PrevJoint;
			Direction.Normalize();
		}
		else
		{
			Direction = (TmpHand.Index.Metacarpal.NextJoint - TmpHand.Index.Metacarpal.PrevJoint);
			Direction.Normalize();
			// Need this offset so the cursor does not show up high in the widget
			// TODO expose the cursor height as a var
			Direction = Direction - 0.6 * (FVector(0, 0, 1));
		}

		
		TargetTrans.SetRotation(Direction.Rotation().Quaternion());

		//Interp is needed to reduce the jitter
		FTransform NewTransform = UKismetMathLibrary::TInterpTo(GetComponentTransform(), TargetTrans, InterpolationDelta, InterpolationSpeed);
		// This will set this component's transform with the more stable interp

		FHitResult SweepHitResult;
		K2_SetWorldTransform(NewTransform, true, SweepHitResult, true);
	
		// Set the sphere location in the widget using the hit result inherited from the parent class
		StaticMesh->SetWorldLocation(LastHitResult.ImpactPoint);

		float Dist = FVector::Dist(CursorLocation, LastHitResult.ImpactPoint);
		if (WidgetInteraction == EUIType::NEAR)
		{
			// 6 cm is the distance between the finger base to the finger tip
			if (Dist < (IndexDitanceFromUI + 6.0f))
			{
				NearClickLeftMouse(TmpHand.HandType);
			}
			// added 2 cm, cause of the jitter can cause accidental release
			else if (Dist > (IndexDitanceFromUI + 8.0f))
			{
				NearReleaseLeftMouse(TmpHand.HandType);
			}
		}
		// In auto mode, automatically enable FAR or NEAR interactions depending on the distance 
		// between the hand and the widget
		HandleAutoMode(Dist);

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
	LeapSubsystem = ULeapSubsystem::Get();
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
		LeapSubsystem->OnLeapPinchMulti.AddUObject(this, &ULeapWidgetInteractionComponent::OnLeapPinch);
		LeapSubsystem->OnLeapUnPinchMulti.AddUObject(this, &ULeapWidgetInteractionComponent::OnLeapUnPinch);
	}
	// Will need to get tracking data regardless of the interaction type (near or far)
	LeapSubsystem->OnLeapFrameMulti.AddUObject(this, &ULeapWidgetInteractionComponent::OnLeapTrackingData);

	LeapSubsystem->SetUsePawnOrigin(true, LeapPawn);
	
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
	if (LeapHandType == EHandType::LEAP_HAND_LEFT)
	{
		PointerIndex = 0;
	}
	else
	{
		PointerIndex = 1;
	}
}

void ULeapWidgetInteractionComponent::OnLeapPinch(const FLeapHandData& HandData)
{
	if (HandData.HandType == LeapHandType && !bIsPinched && WidgetInteraction == EUIType::FAR)
	{
		ScaleUpAndClickButton();
		bIsPinched = true;
	}
}

void ULeapWidgetInteractionComponent::OnLeapUnPinch(const FLeapHandData& HandData)
{
	if (HandData.HandType == LeapHandType && bIsPinched && WidgetInteraction == EUIType::FAR)
	{
		ScaleDownAndUnClickButton();
		bIsPinched = false;
	}	
}

void ULeapWidgetInteractionComponent::NearClickLeftMouse(TEnumAsByte<EHandType> HandType)
{
	if (!bHandTouchWidget && HandType == LeapHandType)
	{
		ScaleUpAndClickButton();
		bHandTouchWidget = true;
	}
}

void ULeapWidgetInteractionComponent::NearReleaseLeftMouse(TEnumAsByte<EHandType> HandType)
{
	if (bHandTouchWidget && HandType == LeapHandType)
	{
		ScaleDownAndUnClickButton();
		bHandTouchWidget = false;
	}
}

void ULeapWidgetInteractionComponent::ScaleUpAndClickButton(const FKey Button)
{
	if (StaticMesh!=nullptr)
	{
		// Scale the cursor by 1/2
		FVector Scale = CursorSize * FVector(1, 1, 1);
		Scale = Scale / 2;
		StaticMesh->SetWorldScale3D(Scale);
	}
	// Press the LeftMouseButton
	PressPointerKey(Button);
}

void ULeapWidgetInteractionComponent::ScaleDownAndUnClickButton(const FKey Button)
{
	if (StaticMesh!=nullptr)
	{
		// Scale the cursor by 2
		FVector Scale = StaticMesh->GetComponentScale();
		if (FMath::IsNearlyEqual(Scale.X, (CursorSize / 2), 1.0E-2F))
		{
			Scale = Scale * 2;
			StaticMesh->SetWorldScale3D(Scale);
		}
	}
	// Release the LeftMouseButton
	ReleasePointerKey(Button);
}

#if WITH_EDITOR
void ULeapWidgetInteractionComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (WidgetInteraction == EUIType::NEAR)
	{
		//Set interpolation speed to 20 for faster cursor movment for near interactions
		InterpolationSpeed = 20.0f;
		// Set the WidgetInteraction type to far when the distance is more than 30
		if (InteractionDistance > 30 && !bAutoMode)
		{
			WidgetInteraction = EUIType::FAR;
		}
	}
}
#endif

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
		LeapSubsystem->SetUsePawnOrigin(false, nullptr);
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
