// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

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
