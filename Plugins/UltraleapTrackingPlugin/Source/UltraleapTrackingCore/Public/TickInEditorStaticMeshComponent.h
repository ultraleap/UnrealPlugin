/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"

#include "TickInEditorStaticMeshComponent.generated.h"

/**
 *
 */
UCLASS()
class ULTRALEAPTRACKING_API UTickInEditorStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UTickInEditorStaticMeshComponent(const FObjectInitializer& PCIP);
};
