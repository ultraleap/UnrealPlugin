/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "UltraleapIEFunctionLibrary.h"

#include "Engine/World.h"

TArray<USkeletalBodySetup*> UUltraleapIEFunctionLibrary::GetSkeletalBodySetups(UPhysicsAsset* PhysicsAsset)
{
	return PhysicsAsset->SkeletalBodySetups;
}
bool UUltraleapIEFunctionLibrary::EnableBodyBoundsByName(
	UPhysicsAsset* PhysicsAsset, const FName& BoneName, const bool Enable, const bool Update)
{
	if (PhysicsAsset == nullptr)
	{
		return false;
	}
	const int32 BodyIndex = PhysicsAsset->FindBodyIndex(BoneName);

	if (BodyIndex < 0)
	{
		return false;
	}
	check(PhysicsAsset->SkeletalBodySetups.IsValidIndex(BodyIndex));
	PhysicsAsset->SkeletalBodySetups[BodyIndex]->bConsiderForBounds = Enable;

	if (Update)
	{
		PhysicsAsset->UpdateBoundsBodiesArray();
	}

	return true;
}

bool UUltraleapIEFunctionLibrary::EnableBodyCollisionByName(
	UPhysicsAsset* PhysicsAsset, const FName& BoneName, const EBodyCollisionResponse::Type BodyCollisionResponse)
{
	if (PhysicsAsset == nullptr)
	{
		return false;
	}
	const int32 BodyIndex = PhysicsAsset->FindBodyIndex(BoneName);

	if (BodyIndex < 0)
	{
		return false;
	}
	const int32 PrimitiveIndex = 0;
	check(PhysicsAsset->SkeletalBodySetups.IsValidIndex(BodyIndex));

	PhysicsAsset->SkeletalBodySetups[BodyIndex]->CollisionReponse = BodyCollisionResponse;
	return true;
}
void UUltraleapIEFunctionLibrary::UpdateBoundsBodiesArray(UPhysicsAsset* PhysicsAsset)
{
	if (PhysicsAsset == nullptr)
	{
		return;
	}
	PhysicsAsset->UpdateBoundsBodiesArray();
}
FName UUltraleapIEFunctionLibrary::GetBodyName(UPhysicsAsset* PhysicsAsset, const int32 BodyIndex)
{
	if (PhysicsAsset == nullptr)
	{
		return FName();
	}

	return PhysicsAsset->SkeletalBodySetups[BodyIndex]->BoneName;
}
void UUltraleapIEFunctionLibrary::InitPhysicsConstraint(UPhysicsConstraintComponent* PhysicsConstraintComponent)
{
	if (PhysicsConstraintComponent == nullptr)
	{
		return;
	}
	PhysicsConstraintComponent->ConstraintInstance.DisableProjection();

	PhysicsConstraintComponent->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = true;
	PhysicsConstraintComponent->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = 100.0f;
}