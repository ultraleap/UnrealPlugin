// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionEngine/IEPhysicsTickStaticMeshComponent.h"

UIEPhysicsTickStaticMeshComponent::UIEPhysicsTickStaticMeshComponent()
{
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	OnCalculateCustomPhysics.BindUObject(this, &UIEPhysicsTickStaticMeshComponent::SubstepTick);
}
void UIEPhysicsTickStaticMeshComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Add custom physics forces substepping ticks
	GetBodyInstance()->AddCustomPhysics(OnCalculateCustomPhysics);
}
void UIEPhysicsTickStaticMeshComponent::SubstepTick(float DeltaTime, FBodyInstance* InBodyInstance)
{
	IEPhysicsTickNotify.Broadcast(DeltaTime, *InBodyInstance);
	/* example call allowed from substep
	FPhysicsInterface::AddImpulse_AssumesLocked(GetBodyInstance()->GetPhysicsActorHandle(), FVector(0.0f, 0.f, force) * DeltaTime);*/
}
void UIEPhysicsTickStaticMeshComponent::DoPhysics(float DeltaTime, bool InSubstep)
{
}