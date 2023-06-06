/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "UltraleapIEFunctionLibrary.generated.h"

/**
 *
 *
 *
 *  Helpers to customise PhysicsAsset behaviours
 *	The Physics asset does half of the job of the SoftContact system in Unity
 *	It builds per bone collisions in a variety of primitive shapes
 *
 *	However, it isn't customisable by blueprint as it's designed to be used at edit time
 *	Functions below map to C++ functionality
 *
 *
 */
UCLASS()
class ULTRALEAPTRACKING_API UUltraleapIEFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**  Get all bodies (body = bone in our case, each bone having collisions generated for it) */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static TArray<USkeletalBodySetup*> GetSkeletalBodySetups(UPhysicsAsset* PhysicsAsset);

	/**  Enable/disable a given body's bounds */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static bool EnableBodyBoundsByName(UPhysicsAsset* PhysicsAsset, const FName& BoneName, const bool Enable, const bool Update);

	/** Enable/disable a given body's collisions */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static bool EnableBodyCollisionByName(
		UPhysicsAsset* PhysicsAsset, const FName& BoneName, const EBodyCollisionResponse::Type BodyCollisionResponse);

	/**  Optimisation, once setting a lot of bounds up, update the state of the PhysicsAsset */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static void UpdateBoundsBodiesArray(UPhysicsAsset* PhysicsAsset);

	/**  Helper, get the body name by body index (=bone name vs index into USkeletalBodySetups) */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static FName GetBodyName(UPhysicsAsset* PhysicsAsset, const int32 BodyIndex);

	/** Helper, initialise a physics constraint (needed as not all params are blueprint writeable) */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	static void InitPhysicsConstraint(UPhysicsConstraintComponent* PhysicsConstraintComponent);
};
