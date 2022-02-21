/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "TickInEditorStaticMeshComponent.h"

UTickInEditorStaticMeshComponent::UTickInEditorStaticMeshComponent(const FObjectInitializer& PCIP) : Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	bHiddenInGame = true;
#if WITH_EDITOR
	bVisualizeComponent = false;
#endif	  // WITH_EDITOR
}