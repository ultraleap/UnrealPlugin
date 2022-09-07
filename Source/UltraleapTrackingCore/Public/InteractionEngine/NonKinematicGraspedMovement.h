/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GraspedMovementHandler.h"
#include "Curves/CurveFloat.h"
#include "NonKinematicGraspedMovement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNonKinematicGraspedMovement : public UGraspedMovementHandler
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNonKinematicGraspedMovement();

protected:
	
public:	

protected:
	// override for specific implementations
	virtual void MoveToImpl(const FVector& SolvedPosition, const FQuat& SolvedRotation, UPrimitiveComponent* RigidBody,
		const bool JustGrasped) override;
	
	// Curve for how fast the physics tracks the desired target location
	UPROPERTY()
	UCurveFloat* StrengthByDistance;

private:
	

	FVector LastSolvedCoMPosition = FVector::ZeroVector;
		
};
