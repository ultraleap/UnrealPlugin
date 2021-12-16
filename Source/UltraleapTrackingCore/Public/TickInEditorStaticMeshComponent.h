// Copyright (C) Ultraleap, Inc. 2011-2021.

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
