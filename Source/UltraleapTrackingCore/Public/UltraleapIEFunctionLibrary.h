// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UltraleapIEFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class ULTRALEAPTRACKING_API UUltraleapIEFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** [PhysX Only] The maximum velocity used to depenetrate this object */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Tracking Functions")
	static void SetMaxDepenetration(UPrimitiveComponent* PrimitiveComponent, float MaxDepenetrationVelocity);
};
