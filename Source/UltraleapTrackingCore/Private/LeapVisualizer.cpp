/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapVisualizer.h"
#include "LeapUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"

// Sets default values
ALeapVisualizer::ALeapVisualizer()
    : NSPlayerAreaBounds(nullptr), NSPTeleportRing(nullptr), Root(nullptr), PlayerAreaBoundsComponent(nullptr), TeleportRingComponent(nullptr), LeapPawn(nullptr), PlayerCameraManager(nullptr), World(nullptr)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = Root;

    NSPlayerAreaBounds = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,
                                                               TEXT("NiagaraSystem'/UltraleapTracking/InteractionEngine/VFX/Leap_NS_PlayAreaBounds.Leap_NS_PlayAreaBounds'")));
    if (NSPlayerAreaBounds == nullptr)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("NSPlayerAreaBounds is nullptr in ALeapVisualizer()"));
    }

    NSPTeleportRing = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,
                                                            TEXT("NiagaraSystem'/UltraleapTracking/InteractionEngine/VFX/Leap_NS_TeleportRing.Leap_NS_TeleportRing'")));

    if (NSPTeleportRing == nullptr)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("NSPTeleportRing is nullptr in ALeapVisualizer()"));
    }

    PlayerAreaBoundsComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraAreaBoundsComponent"));
    TeleportRingComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraTeleportRingComponent"));

    if (!PlayerAreaBoundsComponent || !TeleportRingComponent)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("NSPTeleportRing is nullptr in ALeapVisualizer()"));
    }

    if (RootComponent)
    {
        PlayerAreaBoundsComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        TeleportRingComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    }

    PlayerAreaBoundsComponent->SetAsset(NSPlayerAreaBounds);
    TeleportRingComponent->SetAsset(NSPTeleportRing);
}

// Called when the game starts or when spawned
void ALeapVisualizer::BeginPlay()
{
    Super::BeginPlay();

    FVector2D PlayerArea = UHeadMountedDisplayFunctionLibrary::GetPlayAreaBounds();
    FVector PlayerVec = FVector(PlayerArea.X, PlayerArea.Y, 0.0f);
    PlayerAreaBoundsComponent->SetVariableVec3(FName(*FString("User.PlayAreaBounds")), PlayerVec);

    World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("World is nullptr in ALeapVisualizer::BeginPlay"));
        return;
    }

    LeapPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (LeapPawn == nullptr)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapPawn is nullptr in ALeapVisualizer::BeginPlay"));
        return;
    }
    PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
    if (PlayerCameraManager == nullptr)
    {
        UE_LOG(UltraleapTrackingLog, Error, TEXT("PlayerCameraManager is nullptr in ALeapVisualizer::BeginPlay"));
        return;
    }
}
