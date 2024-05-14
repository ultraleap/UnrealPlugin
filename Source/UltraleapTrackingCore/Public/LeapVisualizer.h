/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4)
#include "Engine/World.h"
#endif

#include "LeapVisualizer.generated.h"

/**
 * This Actor can be used to load Niagara particle systems in C++
 */

UCLASS()
class ULTRALEAPTRACKING_API ALeapVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALeapVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


private:

	UNiagaraSystem* NSPlayerAreaBounds;
	UNiagaraSystem* NSPTeleportRing;

	USceneComponent* Root;

	UNiagaraComponent* PlayerAreaBoundsComponent;
	UNiagaraComponent* TeleportRingComponent;

	APawn* LeapPawn;
	APlayerCameraManager* PlayerCameraManager;
	UWorld* World;

};
