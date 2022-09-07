/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "InteractionEngine/NonKinematicGraspedMovement.h"

/// <summary>
/// This implementation of UGraspedMovementHandler moves a grabbable object to its
/// target position and rotation by setting its rigidbody's velocity and angular
/// velocity such that it will reach the target position and rotation on the next
/// physics update.
/// </summary>
/// 
// Sets default values for this component's properties
UNonKinematicGraspedMovement::UNonKinematicGraspedMovement() : UGraspedMovementHandler()
{
	StrengthByDistance = NewObject<UCurveFloat>();
	StrengthByDistance->FloatCurve.AddKey(0.0f,1.0f);
	StrengthByDistance->FloatCurve.AddKey(2, 0.3f);

}

void UNonKinematicGraspedMovement::MoveToImpl(
	const FVector& SolvedPosition, const FQuat& SolvedRotation, UPrimitiveComponent* RigidBody, const bool JustGrasped)
{
	// centre of mass is in world coords
	FVector RelativeCenterOfMass = RigidBody->GetComponentLocation() - RigidBody->GetCenterOfMass();

	FVector SolvedCenterOfMass = SolvedRotation * RelativeCenterOfMass + SolvedPosition;
	FVector CurrCenterOfMass = FQuat(RigidBody->GetComponentRotation()) * RelativeCenterOfMass + RigidBody->GetComponentLocation();

	FVector TargetVelocity = ToLinearVelocity(CurrCenterOfMass, SolvedCenterOfMass, GetWorld()->GetDeltaSeconds());
	
	FVector TargetAngularVelocity =
		ToAngularVelocity(FQuat(RigidBody->GetComponentRotation()), SolvedRotation, GetWorld()->GetDeltaSeconds());
	
	// Clamp TargetVelocity by MaxVelocity.
	float MaxScaledVelocity = MaxVelocity * SimulationScale;
	float TargetSpeedSqrd = TargetVelocity.SizeSquared();
	if (TargetSpeedSqrd > MaxScaledVelocity * MaxScaledVelocity)
	{
		float TargetPercent = MaxScaledVelocity / FMath::Sqrt(TargetSpeedSqrd);
		TargetVelocity *= TargetPercent;
		TargetAngularVelocity *= TargetPercent;
	}

	
	if (!JustGrasped)
	{
		float RemainingDistanceLastFrame = FVector::Distance(LastSolvedCoMPosition, CurrCenterOfMass);
		FollowStrength = StrengthByDistance->GetFloatValue(RemainingDistanceLastFrame / SimulationScale);
	}

	FVector LerpedVelocity = FMath::Lerp(RigidBody->GetPhysicsLinearVelocity(), TargetVelocity, FollowStrength);
	FVector LerpedAngularVelocity = FMath::Lerp(RigidBody->GetPhysicsAngularVelocityInDegrees(), TargetAngularVelocity, FollowStrength);

	RigidBody->SetPhysicsLinearVelocity(LerpedVelocity);
	RigidBody->SetPhysicsAngularVelocityInDegrees(LerpedAngularVelocity);

	LastSolvedCoMPosition = SolvedCenterOfMass;
}
