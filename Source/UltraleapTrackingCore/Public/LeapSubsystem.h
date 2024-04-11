/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "UltraleapTrackingData.h"
#include "GameFramework/Pawn.h"
#include "LeapSubsystem.generated.h"

//// Blueprint event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLeapGrab, AActor*, GrabbedActor, USkeletalMeshComponent*, LeftHand, USkeletalMeshComponent*, RightHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLeapRelease, AActor*, ReleasedActor, USkeletalMeshComponent*, HandLeft, USkeletalMeshComponent*, HandRight, FName, BoneName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeapGrabAction, FVector, Location, FVector, ForwardVec);

DECLARE_MULTICAST_DELEGATE_OneParam(FLeapHand, const FLeapHandData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FLeapFrame, const FLeapFrameData&);


DECLARE_MULTICAST_DELEGATE_ThreeParams(FLeapGrabNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*);
DECLARE_MULTICAST_DELEGATE_FourParams(FLeapReleaseNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*, FName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FLeapGrabActionNative, FVector, FVector);


/**
 * This subsystem have access to leap events and hand data
 */
UCLASS()
class ULTRALEAPTRACKING_API ULeapSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	ULeapSubsystem();

	static ULeapSubsystem* Get();

	/** Called when grabbing is called*/
	UPROPERTY(BlueprintAssignable, Category = "Leap grab Events")
	FLeapGrab OnLeapGrab;

	
	/** Called when release is called*/
	UPROPERTY(BlueprintAssignable, Category = "Leap release Events")
	FLeapRelease OnLeapRelease;

	UPROPERTY(BlueprintAssignable, Category = "Leap release Events")
	FLeapGrabAction OnLeapGrabAction;

	FLeapGrabActionNative OnLeapGrabActionNative;
	FLeapGrabNative OnLeapGrabNative;
	FLeapReleaseNative OnLeapReleaseNative;

	FLeapHand OnLeapPinchMulti;
	FLeapHand OnLeapUnPinchMulti;
	FLeapFrame OnLeapFrameMulti;

	

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void GrabActionCall(FVector Location, FVector ForwardVec);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapTrackingDataCall(const FLeapFrameData& Frame);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapPinchCall(const FLeapHandData& HandData);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapUnPinchCall(const FLeapHandData& HandData);

	bool GetUseOpenXR();

	void SetUseOpenXR(bool UseXR);

	void SetUsePawnOrigin(bool UseOrigin, APawn* Pawn)
	{
		bUseDeviceOrigin = UseOrigin;
		if (Pawn!=nullptr)
		{
			LeapPawn = Pawn;
		}
	}

private:

	bool bUseOpenXR;

	bool bUseDeviceOrigin;

	APawn* LeapPawn;
};
