/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "InteractionEngine/GrabClassifierComponent.h"

#include "Components/PrimitiveComponent.h"
// Sets default values for this component's properties
UIEGrabClassifierComponent::UIEGrabClassifierComponent()
{
	// no need for tick, this is driven by calls from the IEGrabberComponent tick
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UIEGrabClassifierComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UIEGrabClassifierComponent::UpdateClassifier(const USceneComponent* Hand, const TArray<UGrabClassifierProbe*>& Probes,
	const bool IgnoreTemporal, const bool IsLeftHand, const float DeltaTime, const bool IsGrabbed)
{
	// Store actual minimum curl in case we override it with the ignoreTemporal flag.
	float TempMinCurl = Params.MinimumCurl;
	if (IgnoreTemporal)
	{
		Params.MinimumCurl = -1.0f;
	}
	int32 ProbeIndex = 0;

	// For each probe (fingertip)
	for (auto Probe : Probes)
	{
		// Calculate how extended the finger is
		//	float TempCurl = FVector::DotProduct(Probe->Direction, (ProbeIndex != 0) ? Hand->GetForwardVector() : (IsLeftHand ? 1.0f
		//: -1.0f) * Hand->GetRightVector());
		float TempCurl =
			FVector::DotProduct(Probe->Direction, (ProbeIndex != 0) ? Hand->GetForwardVector() : (-1.0f) * Hand->GetRightVector());

		float CurlVelocity = TempCurl - Probe->PrevCurl;
		Probe->PrevCurl = TempCurl;

		// Determine if this probe is intersecting an object
		bool CollidingWithObject = false;
		if (Probe->CandidateColliders.Num())
		{
			for (auto Collider : Probe->CandidateColliders)
			{
				// UE_LOG(UltraleapTrackingLog, Log, TEXT("Candidate Collider Probe %d. %s"), ProbeIndex, *Collider->GetName());
				auto PrimitiveComponent = Cast<UPrimitiveComponent>(Collider);
				if (!PrimitiveComponent)
				{
					PrimitiveComponent = Cast<UPrimitiveComponent>(Collider->GetAttachParent());
				}

				if (PrimitiveComponent != nullptr)
				{
					if (PrimitiveComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
					{
						CollidingWithObject = true;
						break;
					}
				}
			}
		}
		// if were grabbed in multigrasp, there may be no collision candidates, but we still want to keep grabbing
		// until uncurl occurs
		else if (IsThisControllerGrabbing)
		{
			CollidingWithObject = true;
		}

		// Nullify above findings if fingers are extended
		float ConditionalMaxCurlVelocity = (IsGrabbed ? Params.GrabbedMaximumCurlVelocity : Params.MaximumCurlVelocity);

		CollidingWithObject = CollidingWithObject && (TempCurl < Params.MaximumCurl) && (TempCurl > Params.MinimumCurl) &&
							  (IgnoreTemporal || CurlVelocity < ConditionalMaxCurlVelocity);
		// Probes go inside when they intersect, probes come out when they uncurl
		if (!Probe->IsInside)
		{
			Probe->IsInside = CollidingWithObject;
			Probe->Curl = TempCurl + (ProbeIndex == 0 ? Params.ThumbStickiness : Params.FingerStickiness);

			if (IgnoreTemporal)
			{
				Probe->Curl = (ProbeIndex == 0 ? Params.ThumbStickiness : Params.FingerStickiness);
			}
		}
		else
		{
			if (TempCurl > Probe->Curl)
			{
				Probe->IsInside = CollidingWithObject;
			}
		}
		ProbeIndex++;
	}

	// If thumb and one other finger is "inside" the object, it's a grab!
	// This is the trick!
	IsThisControllerGrabbing =
		(Probes[0]->IsInside && (Probes[1]->IsInside || Probes[2]->IsInside || Probes[3]->IsInside || Probes[4]->IsInside));

	NumInside = Probes[0]->IsInside + Probes[1]->IsInside + Probes[2]->IsInside + Probes[3]->IsInside + Probes[4]->IsInside;

	// If grabbing within 10 frames of releasing, discard grab.
	// Suppresses spurious regrabs and makes throws work better.
	if (CoolDownProgress <= Params.GrabCooldown && !IgnoreTemporal && Params.UseGrabCooldown)
	{
		if (IsThisControllerGrabbing)
		{
			IsThisControllerGrabbing = false;
		}
		CoolDownProgress += DeltaTime;
	}

	// Determine if the object is near the hand or if it's too far away
	/* SHOULD be already filtered in blueprint
	if (IsThisControllerGrabbing && !PrevThisControllerGrabbing)
	{
		bool NearObject = false;
		numberOfColliders[5] = Physics.OverlapSphereNonAlloc(classifier.handGrabCenter ,
			Params.MaximumDistanceFromHand,
			collidingCandidates[5], grabParameters.LAYER_MASK, grabParameters.GRAB_TRIGGERS);
		for (int i = 0; i < numberOfColliders[5]; i++)
		{
			if (collidingCandidates[5][i].attachedRigidbody != null &&
				collidingCandidates[5][i].attachedRigidbody == classifier.body)
			{
				NearObject = true;
				break;
			}
		}

		if (!NearObject)
		{
			IsThisControllerGrabbing = false;
			Probes[0]->IsInside = false;
		}
	}
*/

	// Reset the minimum curl parameter if we modified it due to the ignoreTemporal
	// flag.
	if (IgnoreTemporal)
	{
		Params.MinimumCurl = TempMinCurl;
	}
	NotifyControllerGrabbing();
}
void UIEGrabClassifierComponent::NotifyControllerGrabbing()
{
	// Determine whether there was a state change.
	bool DidStateChange = false;
	if (!PrevThisControllerGrabbing && IsThisControllerGrabbing /* && graspMode == GraspUpdateMode.BeginGrasp*/)
	{
		DidStateChange = true;

		PrevThisControllerGrabbing = IsThisControllerGrabbing;
	}
	else if (PrevThisControllerGrabbing && !IsThisControllerGrabbing /* &&
			 interactionHand.graspedObject == behaviour && graspMode == GraspUpdateMode.ReleaseGrasp*/)
	{
		DidStateChange = true;

		CoolDownProgress = 0.0f;
		PrevThisControllerGrabbing = IsThisControllerGrabbing;
	}

	if (DidStateChange)
	{
		OnIsGrabbingChanged.Broadcast(this, IsThisControllerGrabbing);
	}
}
void UIEGrabClassifierComponent::ForceReset()
{
	PrevThisControllerGrabbing = IsThisControllerGrabbing = false;
}
