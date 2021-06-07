// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TickInEditorStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class LEAPMOTION_API UTickInEditorStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UTickInEditorStaticMeshComponent(const FObjectInitializer& PCIP);
};
