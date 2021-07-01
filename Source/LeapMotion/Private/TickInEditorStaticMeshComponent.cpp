// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.


#include "TickInEditorStaticMeshComponent.h"

UTickInEditorStaticMeshComponent::UTickInEditorStaticMeshComponent(const FObjectInitializer& PCIP) : Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	bHiddenInGame = true;
#if WITH_EDITOR
	bVisualizeComponent = false;
#endif //WITH_EDITOR
}