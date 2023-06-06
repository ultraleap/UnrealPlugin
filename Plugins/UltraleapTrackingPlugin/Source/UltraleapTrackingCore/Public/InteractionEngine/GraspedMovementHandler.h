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
#include "GraspedMovementHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UGraspedMovementHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGraspedMovementHandler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Helper, port of MoveTo() - updates grabbed primitive's velocity base on desired target
	 * location*/
	UFUNCTION(BlueprintCallable, Category = "Ultraleap Interaction Engine")
	void MoveTo(const FVector& SolvedPosition, const FRotator& SolvedRotation, UPrimitiveComponent* RigidBody, const bool JustGrasped)
	{
		if (!RigidBody)
		{
			return;
		}
		FQuat QuatSolvedRotation(SolvedRotation);
		MoveToImpl(SolvedPosition, QuatSolvedRotation, RigidBody, JustGrasped);
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float FollowStrength = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float MaxVelocity = 300.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float SimulationScale = 1.0f;

protected:
	// override for specific implementations
	virtual void MoveToImpl(
		const FVector& SolvedPosition, const FQuat& SolvedRotation, UPrimitiveComponent* RigidBody, const bool JustGrasped)
	{
		
	}
		
	
    static FVector ToLinearVelocity(const FVector& DeltaPosition,const float DeltaTime)
	{
		return DeltaPosition / DeltaTime;
	}

	static FVector ToLinearVelocity(const FVector& StartPosition,const FVector& DestinationPosition,const float DeltaTime)
	{
		return ToLinearVelocity(DestinationPosition - StartPosition, DeltaTime);
	}

	static FVector ToAngularVelocity(const FQuat& DeltaRotation,const float DeltaTime)
	{
		FVector DeltaAxis;
		float DeltaAngle;
		DeltaRotation.ToAxisAndAngle(DeltaAxis, DeltaAngle);

		if (!FMath::IsFinite(DeltaAxis.X))
		{
			DeltaAxis = FVector::ZeroVector;
			DeltaAngle = 0;
		}

		if (DeltaAngle > 180)
		{
			DeltaAngle -= 360.0f;
		}

		return DeltaAxis * FMath::DegreesToRadians(DeltaAngle) / DeltaTime;
	}

	static FVector ToAngularVelocity(const FQuat& StartRotation, const FQuat DestinationRotation,const float DeltaTime)
	{
		return ToAngularVelocity(DestinationRotation * StartRotation.Inverse(), DeltaTime);
	}
};
