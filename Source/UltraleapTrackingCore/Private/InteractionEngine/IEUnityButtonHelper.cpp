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

FVector UIEUnityButtonHelper::constrainDepressedLocalPosition(const FVector& initialLocalPosition, const FVector& localPosition)
{
	// Buttons are only allowed to move along their Z axis.
	return FVector(initialLocalPosition.X, initialLocalPosition.Y, _localPhysicsPosition.Z + localPosition.Z);
}

void UIEUnityButtonHelper::Update(
	UPARAM(Ref) bool& ignoreGrasping, UPARAM(Ref) bool& _initialIgnoreGrasping, const bool& isPrimaryHovered, const bool& isGrasped,
	const bool& controlEnabled, UPARAM(Ref) bool& ignoreContact, UPrimitiveComponent* rigidbody,
	const FRotator& _initialLocalRotation, const float primaryHoverDistance, 
	const float _springForce, const FVector2D& minMaxHeight, const float restingHeight, const float WorldDelta, const FVector& initialLocalPosition, UPARAM(Ref) float& _pressedAmount, USceneComponent* primaryHoveringController)
{
	// Reset our convenience state variables.
	_pressedThisFrame = false;
	 _unpressedThisFrame = false;

	// Disable collision on this button if it is not the primary hover.
	ignoreGrasping = _initialIgnoreGrasping ? true : !isPrimaryHovered && !isGrasped;
	ignoreContact = (!isPrimaryHovered || isGrasped) || !controlEnabled;

	// Enforce local rotation (if button is child of non-kinematic rigidbody,
	// this is necessary).
	rigidbody->SetRelativeRotation(_initialLocalRotation);
	
	// Record and enforce the sliding state from the previous frame.
	if (primaryHoverDistance < 0.005f || isGrasped || _isPressed)
	{
		_localPhysicsPosition = constrainDepressedLocalPosition(initialLocalPosition,
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(rigidbody->GetComponentLocation()) - _localPhysicsPosition);
	}
	else
	{
		FVector2D localSlidePosition = FVector2D(_localPhysicsPosition.X, _localPhysicsPosition.Y);

		_localPhysicsPosition =
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(rigidbody->GetComponentLocation());

		_localPhysicsPosition = FVector(localSlidePosition.X, localSlidePosition.Y, _localPhysicsPosition.Z);
	}
	bool HasVelocity = false;
	// Calculate the physical kinematics of the button in local space
	FVector localPhysicsVelocity = rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformVector(rigidbody->GetPhysicsLinearVelocity());
	if (_isPressed && isPrimaryHovered && _lastDepressor != nullptr)
	{
		FVector curLocalDepressorPos =
			rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(_lastDepressor->GetComponentLocation());
		FVector origLocalDepressorPos = rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(
			rigidbody->GetComponentTransform().TransformPosition(_localDepressorPosition));
		localPhysicsVelocity = FVector::BackwardVector * 0.05f;
		_localPhysicsPosition = constrainDepressedLocalPosition(initialLocalPosition, curLocalDepressorPos - origLocalDepressorPos);
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
								_localPhysicsPosition.Z),
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
	_physicsPosition = rigidbody->GetAttachParent()->GetComponentTransform().TransformPosition(_localPhysicsPosition);
	_physicsVelocity = rigidbody->GetAttachParent()->GetComponentTransform().TransformVector(localPhysicsVelocity);

	// Calculate the Depression State of the Button from its Physical Position
	// Set its Graphical Position to be Constrained Physically
	bool oldDepressed = _isPressed;

	// Normalized depression amount.
	_pressedAmount = FMath::GetMappedRangeValueClamped(
		FVector2D(initialLocalPosition.Z - minMaxHeight.X,
										  initialLocalPosition.X - FMath::Lerp(minMaxHeight.X, minMaxHeight.Y, restingHeight)),
		FVector2D(1, 0), _localPhysicsPosition.Z);

	if (HasVelocity)
	{
		//Debug.Log("localPhysicsPosition " + localPhysicsPosition.ToString("F3") + " " + _pressedAmount.ToString("F3"));
	}
	// If the button is depressed past its limit...
	if (_localPhysicsPosition.Z > initialLocalPosition.Z - minMaxHeight.X)
	{
		rigidbody->SetRelativeLocation(
			FVector(_localPhysicsPosition.X, _localPhysicsPosition.Y, initialLocalPosition.Z - minMaxHeight.X));
		if ((isPrimaryHovered && _lastDepressor != nullptr) || isGrasped)
		{
			_isPressed = true;
		}
		else
		{
			_physicsPosition = rigidbody->GetAttachParent()->GetComponentTransform().TransformPosition(
				FVector(_localPhysicsPosition.X, _localPhysicsPosition.Y, initialLocalPosition.Z - minMaxHeight.X));
			_physicsVelocity = _physicsVelocity * 0.1f;
			_isPressed = false;
			_lastDepressor = nullptr;
		}
		// Else if the button is extended past its limit...
	}
	else if (_localPhysicsPosition.Z < initialLocalPosition.Z - minMaxHeight.Y)
	{
		rigidbody->SetRelativeLocation(
			FVector(_localPhysicsPosition.X, _localPhysicsPosition.Y, initialLocalPosition.Z - minMaxHeight.Y));
		_physicsPosition = rigidbody->GetComponentLocation();
		_isPressed = false;
		_lastDepressor = nullptr;
	}
	else
	{
		// Else, just make the physical and graphical motion of the button match
		rigidbody->SetRelativeLocation(_localPhysicsPosition);

		// Allow some hysteresis before setting isDepressed to false.
		if (!_isPressed || !(_localPhysicsPosition.Z > initialLocalPosition.Z - (minMaxHeight.Y - minMaxHeight.X) * 0.1F))
		{
			_isPressed = false;
			_lastDepressor = nullptr;
		}
	}

	// If our depression state has changed since last time...
	if (_isPressed && !oldDepressed)
	{
		//primaryHoveringController.primaryHoverLocked = true;
		_lockedInteractingController = primaryHoveringController;

		OnPress();
		_pressedThisFrame = true;
	}
	else if (!_isPressed && oldDepressed)
	{
		_unpressedThisFrame = true;
		OnUnpress();

	/* if (!(isGrasped && graspingController == _lockedInteractingController))
		{
			_lockedInteractingController.primaryHoverLocked = false;
		}
			*/
		_lastDepressor = nullptr;
	}

	_localPhysicsPositionConstrained = rigidbody->GetAttachParent()->GetComponentTransform().InverseTransformPosition(_physicsPosition);
}
