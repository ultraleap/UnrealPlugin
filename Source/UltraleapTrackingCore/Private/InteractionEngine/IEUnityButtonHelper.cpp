// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionEngine/IEUnityButtonHelper.h"

// Sets default values for this component's properties
UIEUnityButtonHelper::UIEUnityButtonHelper()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

const float FRICTION_COEFFICIENT = 30;
const float DRAG_COEFFICIENT = 60;

void UIEUnityButtonHelper::Update(UPARAM(Ref) bool& _pressedThisFrame, UPARAM(Ref) bool& _unpressedThisFrame,
	UPARAM(Ref) bool& ignoreGrasping, UPARAM(Ref) bool& _initialIgnoreGrasping, const bool& isPrimaryHovered, const bool& isGrasped,
	const bool& controlEnabled, UPARAM(Ref) bool& ignoreContact, UPrimitiveComponent* rigidbody,
	UPARAM(Ref) FRotator& _initialLocalRotation, const float primaryHoverDistance, UPARAM(Ref) bool& isPressed,
	UPARAM(Ref) FVector& localPhysicsPosition, UPARAM(Ref) USceneComponent*& _lastDepressor,
	UPARAM(Ref) FVector& _localDepressorPosition, const float _springForce, const FVector2D& minMaxHeight, const float restingHeight, const float WorldDelta, const FVector& initialLocalPosition, UPARAM(Ref) FVector& physicsPosition,
	UPARAM(Ref) FVector& _physicsVelocity, UPARAM(Ref) float& _pressedAmount)
{
	// Reset our convenience state variables.
	_pressedThisFrame = false;
	 _unpressedThisFrame = false;

	// Disable collision on this button if it is not the primary hover.
	ignoreGrasping = _initialIgnoreGrasping ? true : !isPrimaryHovered && !isGrasped;
	ignoreContact = (!isPrimaryHovered || isGrasped) || !controlEnabled;

	// Enforce local rotation (if button is child of non-kinematic rigidbody,
	// this is necessary).
	//transform.localRotation = _initialLocalRotation;
	rigidbody->SetRelativeRotation(_initialLocalRotation);
	
	// Record and enforce the sliding state from the previous frame.
	if (primaryHoverDistance < 0.005f || isGrasped || isPressed)
	{
		localPhysicsPosition = constrainDepressedLocalPosition(
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(rigidbody->GetComponentLocation()) - localPhysicsPosition);
	}
	else
	{
		FVector2D localSlidePosition = FVector2D(localPhysicsPosition.X, localPhysicsPosition.Y);

		localPhysicsPosition =
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(rigidbody->GetComponentLocation());

		localPhysicsPosition = FVector(localSlidePosition.X, localSlidePosition.Y, localPhysicsPosition.Z);
	}
	bool HasVelocity = false;
	// Calculate the physical kinematics of the button in local space
	FVector localPhysicsVelocity = rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformVector(rigidbody->GetPhysicsLinearVelocity());
	if (isPressed && isPrimaryHovered && _lastDepressor != nullptr)
	{
		FVector curLocalDepressorPos =
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(_lastDepressor->GetComponentLocation());
		FVector origLocalDepressorPos = rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(
			rigidbody->GetComponentTransform().TransformPosition(_localDepressorPosition));
		localPhysicsVelocity = FVector::BackwardVector * 0.05f;
		localPhysicsPosition = constrainDepressedLocalPosition(curLocalDepressorPos - origLocalDepressorPos);
	}
	else if (isGrasped)
	{
		// Do nothing!
	}
	else
	{
		FVector originalLocalVelocity = localPhysicsVelocity;

		// Spring force
		localPhysicsVelocity +=
			FMath::Clamp(_springForce * 10000.0f *
					(initialLocalPosition.Z - FMath::Lerp(minMaxHeight.X, minMaxHeight.Y, restingHeight) -
								localPhysicsPosition.Z),
				-100.0f / rigidbody->GetAttachParent()->GetComponentScale().X,
				100.0f / rigidbody->GetAttachParent()->GetComponentScale().X) *
			WorldDelta * FVector::ForwardVector;

		if (FMath::Abs(localPhysicsVelocity.Size()) > 0.001)
		{
			//Debug.Log("LocalPhysicsVelocity step 1 " + localPhysicsVelocity.ToString("F3"));
		}
		// Friction & Drag
		float velMag = originalLocalVelocity.Size();
		float frictionDragVelocityChangeAmt = 0;
		if (velMag > 0)
		{
			// Friction force
			float frictionForceAmt = velMag * FRICTION_COEFFICIENT;
			frictionDragVelocityChangeAmt += WorldDelta * rigidbody->GetAttachParent()->GetComponentScale().X * frictionForceAmt;

			// Drag force
			float velSqrMag = velMag * velMag;
			float dragForceAmt = velSqrMag * DRAG_COEFFICIENT;
			frictionDragVelocityChangeAmt += WorldDelta * rigidbody->GetAttachParent()->GetComponentScale().X * dragForceAmt;

			// Apply velocity change, but don't let friction or drag let velocity
			// magnitude cross zero.
			float newVelMag = FMath::Max<float>(0, velMag - frictionDragVelocityChangeAmt);
			localPhysicsVelocity = localPhysicsVelocity / velMag * newVelMag;
		}

		if (FMath::Abs(localPhysicsVelocity.Size()) > 0.001)
		{
			//Debug.Log("LocalPhysicsVelocity step 2" + localPhysicsVelocity.ToString("F3"));
			HasVelocity = true;
		}
	}

	// Transform the local physics back into world space
	physicsPosition = rigidbody->GetAttachParent()->GetComponentTransform().TransformPosition(localPhysicsPosition);
	_physicsVelocity = rigidbody->GetAttachParent()->GetComponentTransform().TransformVector(localPhysicsVelocity);

	// Calculate the Depression State of the Button from its Physical Position
	// Set its Graphical Position to be Constrained Physically
	bool oldDepressed = isPressed;

	// Normalized depression amount.
	// TODO FMath::GetMappedRangeValueClamped
	_pressedAmount = localPhysicsPosition.Z.Map(initialLocalPosition.Z - minMaxHeight.X,
		initialLocalPosition.X - FMath::Lerp(minMaxHeight.X, minMaxHeight.Y, restingHeight), 1, 0);
	if (HasVelocity)
	{
		//Debug.Log("localPhysicsPosition " + localPhysicsPosition.ToString("F3") + " " + _pressedAmount.ToString("F3"));
	}
	// If the button is depressed past its limit...
	if (localPhysicsPosition.Z > initialLocalPosition.Z - minMaxHeight.X)
	{
		rigidbody->SetRelativeLocation(
			FVector(localPhysicsPosition.X, localPhysicsPosition.Y, initialLocalPosition.Z - minMaxHeight.X));
		if ((isPrimaryHovered && _lastDepressor != nullptr) || isGrasped)
		{
			_isPressed = true;
		}
		else
		{
			physicsPosition = transform.parent.TransformPoint(
				new Vector3(localPhysicsPosition.x, localPhysicsPosition.y, initialLocalPosition.z - minMaxHeight.x));
			_physicsVelocity = _physicsVelocity * 0.1f;
			_isPressed = false;
			_lastDepressor = null;
		}
		// Else if the button is extended past its limit...
	}
	else if (localPhysicsPosition.z < initialLocalPosition.z - minMaxHeight.y)
	{
		transform.localPosition =
			new Vector3(localPhysicsPosition.x, localPhysicsPosition.y, initialLocalPosition.z - minMaxHeight.y);
		physicsPosition = transform.position;
		_isPressed = false;
		_lastDepressor = null;
	}
	else
	{
		// Else, just make the physical and graphical motion of the button match
		transform.localPosition = localPhysicsPosition;

		// Allow some hysteresis before setting isDepressed to false.
		if (!isPressed || !(localPhysicsPosition.z > initialLocalPosition.z - (minMaxHeight.y - minMaxHeight.x) * 0.1F))
		{
			_isPressed = false;
			_lastDepressor = null;
		}
	}

	// If our depression state has changed since last time...
	if (isPressed && !oldDepressed)
	{
		primaryHoveringController.primaryHoverLocked = true;
		_lockedInteractingController = primaryHoveringController;

		OnPress();
		_pressedThisFrame = true;
	}
	else if (!isPressed && oldDepressed)
	{
		_unpressedThisFrame = true;
		OnUnpress();

		if (!(isGrasped && graspingController == _lockedInteractingController))
		{
			_lockedInteractingController.primaryHoverLocked = false;
		}

		_lastDepressor = null;
	}

	localPhysicsPositionConstrained = transform.parent.InverseTransformPoint(physicsPosition);
}
