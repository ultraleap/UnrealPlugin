/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "UltraleapTickInEditorBaseActor.h"

// Sets default values
AUltraleapTickInEditorBaseActor::AUltraleapTickInEditorBaseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUltraleapTickInEditorBaseActor::BeginPlay()
{
	Super::BeginPlay();
	
}
 
bool AUltraleapTickInEditorBaseActor::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor && bTickInEditor)
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Called every frame
void AUltraleapTickInEditorBaseActor::Tick(float DeltaTime)
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
	{
#if WITH_EDITOR
		EditorTick(DeltaTime);
#endif
	}
	else
	{
		Super::Tick(DeltaTime);
	}
}

