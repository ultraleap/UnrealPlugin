/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapWidgetInteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "LeapUtility.h"
#include "UObject/ConstructorHelpers.h"

ULeapWidgetInteractionComponent::ULeapWidgetInteractionComponent()
	: LeapHandType(EHandType::LEAP_HAND_LEFT)
	, WidgetInteraction(EUIType::FAR)
	, StaticMesh(nullptr)
	, MaterialBase(nullptr)
	, CursorSize(0.03)
	, bAutoMode(true)
	, IndexDitanceFromUI(0.0f)
	, HandVisibility(false)
	, LeapSubsystem(nullptr)
	, WristRotationFactor(0.0f)
	, InterpolationSpeed(10)
	, YAxisCalibOffset(4.0f)
	, ZAxisCalibOffset(4.0f)
	, LeapPawn(nullptr)
	, PointerActor(nullptr)
	, World(nullptr)
	, LeapDynMaterial(nullptr)
	, PlayerCameraManager(nullptr)
	, bIsPinched(false)
	, bHandTouchWidget(false)
	, bAutoModeTrigger(false)
	, AxisRotOffset(45.0f)
	, TriggerFarOffset(20.0f)
	, FingerJointEstimatedLen(3.5f)
	, ShoulderWidth(15.0f)

{
	CreatStaticMeshForCursor();
}

void ULeapWidgetInteractionComponent::InitCalibrationArrays()
{
	CalibratedHeadRot.Add({0, 0, 0});	 // Look straight
	CalibratedHeadPos.Add({0, 0, 0});

	CalibratedHeadRot.Add({-AxisRotOffset, 0, 0});	  // Look down
	CalibratedHeadPos.Add({0, 0, -ZAxisCalibOffset});

	CalibratedHeadRot.Add({AxisRotOffset, 0, 0});	 // Look Up
	CalibratedHeadPos.Add({0, 0, ZAxisCalibOffset});

	CalibratedHeadRot.Add({0, 0, -AxisRotOffset});	  // Roll left
	CalibratedHeadPos.Add({0, -YAxisCalibOffset, -ZAxisCalibOffset / 2});

	CalibratedHeadRot.Add({0, 0, AxisRotOffset});	 // Roll right
	CalibratedHeadPos.Add({0, YAxisCalibOffset, -ZAxisCalibOffset / 2});
}

ULeapWidgetInteractionComponent::~ULeapWidgetInteractionComponent()
{
	OnRayComponentVisible.Clear();
}

void ULeapWidgetInteractionComponent::CreatStaticMeshForCursor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorMesh"));
	if (StaticMesh == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("StaticMesh is nullptr in CreatStaticMeshForCursor()"));
		return;
	}
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MaterialBase = LoadObject<UMaterial>(
		nullptr, TEXT("Material'/UltraleapTracking/InteractionEngine/Materials/IE2_Materials/M_LaserPointer-Outer.M_LaserPointer-Outer'"));

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetWorldScale3D(CursorSize * FVector::OneVector);
	}
}

void ULeapWidgetInteractionComponent::HandleDistanceChange(float Dist, float MinDistance)
{
	// Adding 20 cm for the distance before we exit near field interactions
	float ExistDistance = MinDistance + TriggerFarOffset;
	if (Dist < MinDistance && WidgetInteraction == EUIType::FAR && !bAutoModeTrigger)
	{
		WidgetInteraction = EUIType::NEAR;

		// Broadcast event when the rays visbility change
		if (OnRayComponentVisible.IsBound())
		{
			OnRayComponentVisible.Broadcast(false);
		}
		bAutoModeTrigger = true;
		if (bAutoMode)
		{
			StaticMesh->SetHiddenInGame(true);
		}
	}
	else if (Dist > ExistDistance && WidgetInteraction == EUIType::NEAR && bAutoModeTrigger)
	{
		WidgetInteraction = EUIType::FAR;
		// Broadcast event when the rays visbility change
		if (OnRayComponentVisible.IsBound())
		{
			OnRayComponentVisible.Broadcast(true);
		}
		bAutoModeTrigger = false;
		if (bAutoMode)
		{
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
		FVector Position = FVector::ZeroVector;
		FVector Direction = FVector();
		FVector IndexDistalN = TmpHand.Index.Distal.NextJoint;
		FVector IndexIntermN = TmpHand.Index.Intermediate.NextJoint;
		FVector IndexMetaN = TmpHand.Index.Metacarpal.NextJoint;

		FRotator ForwardRot = PlayerCameraManager->GetActorForwardVector().Rotation();
		ForwardRot = FRotator(0, ForwardRot.Yaw, 0);
		FVector ForwardDirection = ForwardRot.Vector();

		bool bNear = (WidgetInteraction == EUIType::NEAR);

		Position = bNear ? IndexIntermN : IndexMetaN;
		FVector FilteredPosition = Position;
		Direction = bNear ? (IndexDistalN - IndexIntermN) : GetHandRayDirection(TmpHand, FilteredPosition);
		FVector FilteredDirection = FVector::ZeroVector;

		FTransform TargetTrans = FTransform();

		TargetTrans.SetLocation(FilteredPosition);
		TargetTrans.SetRotation(Direction.Rotation().Quaternion());

		FTransform NewTransform =
			UKismetMathLibrary::TInterpTo(GetComponentTransform(), TargetTrans, World->GetDeltaSeconds(), InterpolationSpeed);

		FHitResult SweepHitResult;
		K2_SetWorldTransform(NewTransform, true, SweepHitResult, true);

		StaticMesh->SetWorldLocation(LastHitResult.ImpactPoint);

		TWeakObjectPtr<AActor> HitActor = nullptr;
#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2)
		HitActor = LastHitResult.GetActor();
#else
		HitActor = LastHitResult.Actor;
#endif
		if (HitActor == nullptr)
		{
			return;
		}
		if (!HitActor->Tags.Contains(FName("UltraleapUMG")))
		{
			return;
		}

		float Dist = FVector::Dist(Position, LastHitResult.ImpactPoint);

		if (WidgetInteraction == EUIType::NEAR)
		{
			// 3.5 cm is the distance between the finger base to the finger tip
			if (Dist < (IndexDitanceFromUI + FingerJointEstimatedLen))
			{
				NearClickLeftMouse(TmpHand.HandType);
			}
			// added 2 cm, cause of the jitter can cause accidental release
			// Also makes better user experience when using sliders
			else if (Dist > (IndexDitanceFromUI + FingerJointEstimatedLen + 2.0f))
			{
				NearReleaseLeftMouse(TmpHand.HandType);
			}
		}
		// In auto mode, automatically enable FAR or NEAR interactions depending on the distance
		// between the hand and the widget
		// Also trigger event when visibility changed		
		if (bAutoMode)
		{
			HandleDistanceChange(Dist);
		}
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("nullptr in DrawLeapCircles"));
	}
}

FVector ULeapWidgetInteractionComponent::GetHandRayDirection(FLeapHandData& TmpHand, FVector& Position)
{
	if (World == nullptr && PlayerCameraManager == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("World or PlayerCameraManager nullptr in GetHandRayDirection"));
		return FVector::ZeroVector;
	}

	// Use neck offset to overcome camera roll and pitch rotations
	FVector NeckOffset = GetNeckOffset();
	// Get the neck postion
	FVector CameraLocationWithNeckOffset = PlayerCameraManager->GetCameraLocation();
	CameraLocationWithNeckOffset -= NeckOffset;
	CameraLocationWithNeckOffset -= 15 * FVector::UpVector;

	// Get the general right direction of the camera
	FRotator RightRot = PlayerCameraManager->GetActorRightVector().Rotation();
	RightRot = FRotator(0, RightRot.Yaw, 0);
	FVector RightDirection = RightRot.Vector();

	// Get shoulders positions, with respect to the neck
	FVector ShoulderPos = FVector::ZeroVector;
	// Use the camera location with offset to overcome head Roll and Pitch
	ShoulderPos = CameraLocationWithNeckOffset;
	ShoulderPos +=  WristRotationFactor * PlayerCameraManager->GetActorForwardVector();
	ShoulderPos += RightDirection * (TmpHand.HandType == EHandType::LEAP_HAND_LEFT ? -ShoulderWidth : ShoulderWidth);
	// Get approximate pintch position
	if (WidgetInteraction == EUIType::FAR)
	{
		Position += FVector::UpVector;
		Position += 6 * PlayerCameraManager->GetActorForwardVector();
		Position += PlayerCameraManager->GetActorRightVector() * (TmpHand.HandType == EHandType::LEAP_HAND_LEFT ? 2 : -2);
	}
	// Get the direction from the shoulders to the pinch position
	FVector Direction = Position - ShoulderPos;

	return Direction;
}

FVector ULeapWidgetInteractionComponent::GetNeckOffset()
{
	if (PlayerCameraManager == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("PlayerCameraManager in GetNeckOffset"));
		return FVector();
	}

	FRotator HMDRotation = PlayerCameraManager->GetCameraRotation();
	float AlphaPitch = 0;
	float AlphaRoll = 0;
	FVector PitchOffset, RollOffset;
	if (HMDRotation.Pitch < 0)
	{
		AlphaPitch = UKismetMathLibrary::NormalizeToRange(-HMDRotation.Pitch, 0, -CalibratedHeadRot[1].Pitch);
		PitchOffset = FMath::Lerp(FVector(0), CalibratedHeadPos[1], AlphaPitch);
	}
	else
	{
		AlphaPitch = UKismetMathLibrary::NormalizeToRange(HMDRotation.Pitch, 0, CalibratedHeadRot[2].Pitch);
		PitchOffset = FMath::Lerp(FVector(0), CalibratedHeadPos[2], AlphaPitch);
	}

	if (HMDRotation.Roll < 0)
	{
		AlphaRoll = UKismetMathLibrary::NormalizeToRange(-HMDRotation.Roll, 0, -CalibratedHeadRot[3].Roll);
		RollOffset = FMath::Lerp(FVector(0), CalibratedHeadPos[3], AlphaRoll);
	}
	else
	{
		AlphaRoll = UKismetMathLibrary::NormalizeToRange(HMDRotation.Roll, 0, CalibratedHeadRot[4].Roll);
		RollOffset = FMath::Lerp(FVector(0), CalibratedHeadPos[4], AlphaRoll);
	}

	FVector Offset = PitchOffset + RollOffset;
	FRotator Rot = FRotator(0, HMDRotation.Yaw, 0);

	return Rot.RotateVector(Offset);
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
	if (LeapSubsystem == nullptr)
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
		FVector Scale = CursorSize * FVector::OneVector;
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

	//Hide on begin play
	StaticMesh->SetHiddenInGame(true);

	// SmoothingOneEuroFilter = ViewportInteractionUtils::FOneEuroFilter(MinCutoff, CutoffSlope, DeltaCutoff);

	InitCalibrationArrays();
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
	if (StaticMesh != nullptr)
	{
		// Scale the cursor by 1/2
		FVector Scale = CursorSize * FVector::OneVector;
		Scale = Scale / 2;
		StaticMesh->SetWorldScale3D(Scale);
	}
	// Press the LeftMouseButton
	PressPointerKey(Button);
}

void ULeapWidgetInteractionComponent::ScaleDownAndUnClickButton(const FKey Button)
{
	if (StaticMesh != nullptr)
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

void ULeapWidgetInteractionComponent::CleanUpEvents()
{
	// Clean up the subsystem events
	if (LeapSubsystem != nullptr)
	{
		LeapSubsystem->OnLeapFrameMulti.Clear();
		LeapSubsystem->OnLeapPinchMulti.Clear();
		LeapSubsystem->OnLeapUnPinchMulti.Clear();
		LeapSubsystem->SetUsePawnOrigin(false, nullptr);
	}
}

void ULeapWidgetInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CleanUpEvents();
}

void ULeapWidgetInteractionComponent::InitializeComponent()
{
}

void ULeapWidgetInteractionComponent::OnLeapTrackingData(const FLeapFrameData& Frame)
{
	TArray<FLeapHandData> Hands = Frame.Hands;
	
	HandleVisibilityChange(Frame);

	for (int32 i = 0; i < Hands.Num(); ++i)
	{
		DrawLeapCursor(Hands[i]);
	}
}

void ULeapWidgetInteractionComponent::HandleVisibilityChange(const FLeapFrameData& Frame)
{
	bool LatestHandVis = LeapHandType == EHandType::LEAP_HAND_LEFT ? Frame.LeftHandVisible : Frame.RightHandVisible;

	if (LatestHandVis != HandVisibility)
	{
		HandVisibility = LatestHandVis;
		StaticMesh->SetHiddenInGame(!HandVisibility);
	}
}
