/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "InteractionEngine/IEPhysicsTickStaticMeshComponent.h"


void FIESecondaryTickFunction::ExecuteTick(
	float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target && !Target->IsPendingKill() && !Target->IsUnreachable())
	{
		FScopeCycleCounterUObject ActorScope(Target);
		Target->IEPostPhysicsTickNotify.Broadcast(DeltaTime);
	}
}

UIEPhysicsTickStaticMeshComponent::UIEPhysicsTickStaticMeshComponent()
{
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SecondaryComponentTick.TickGroup = TG_PostPhysics;
	SecondaryComponentTick.bCanEverTick = true;
	SecondaryComponentTick.bStartWithTickEnabled = true;


	OnCalculateCustomPhysics.BindUObject(this, &UIEPhysicsTickStaticMeshComponent::SubstepTick);
}
void UIEPhysicsTickStaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsTemplate() && SecondaryComponentTick.bCanEverTick)
	{
		SecondaryComponentTick.Target = this;
		SecondaryComponentTick.SetTickFunctionEnable(SecondaryComponentTick.bStartWithTickEnabled);
		SecondaryComponentTick.RegisterTickFunction(GetOwner()->GetLevel());
	}
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