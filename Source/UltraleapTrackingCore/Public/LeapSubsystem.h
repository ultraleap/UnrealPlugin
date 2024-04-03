// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "UltraleapTrackingData.h"
#include "GameFramework/Pawn.h"
#include "LeapSubsystem.generated.h"

//// Blueprint event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLeapGrab, AActor*, GrabbedActor, USkeletalMeshComponent*, LeftHand, USkeletalMeshComponent*, RightHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FLeapRelease, AActor*, ReleasedActor, USkeletalMeshComponent*, HandLeft, USkeletalMeshComponent*, HandRight, FName, BoneName, bool, IsLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeapGrabAction, FVector, Location, FVector, ForwardVec);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapFrame, const FLeapFrameData&, Frame);

DECLARE_MULTICAST_DELEGATE_OneParam(FLeapHand, const FLeapHandData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FLeapFrameNative, const FLeapFrameData&);


DECLARE_MULTICAST_DELEGATE_ThreeParams(FLeapGrabNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*);
DECLARE_MULTICAST_DELEGATE_FiveParams(FLeapReleaseNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*, FName, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FLeapGrabActionNative, FVector, FVector);

// Native C++ events
//DECLARE_DELEGATE_FourParams(FLeapReleaseNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*, FName);
//DECLARE_DELEGATE_ThreeParams(FLeapGrabNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*);
//DECLARE_DELEGATE_OneParam(FLeapFrameSignatureNative, const FLeapFrameData&);
//DECLARE_DELEGATE_OneParam(FLeapHandSignaturenative, const FLeapHandData&);

/**
 * 
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

	UPROPERTY(BlueprintAssignable, Category = "Leap Frame")
	FLeapFrame OnLeapFrame;

	FLeapGrabActionNative OnLeapGrabActionNative;


	FLeapGrabNative OnLeapGrabNative;
	FLeapReleaseNative OnLeapReleaseNative;

	FLeapHand OnLeapPinchMulti;
	FLeapHand OnLeapUnPinchMulti;
	FLeapFrameNative OnLeapFrameMulti;

	

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnReleaseCall(
		AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName, bool IsLeft = true);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void GrabActionCall(FVector Location, FVector ForwardVec);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapTrackingDataCall(const FLeapFrameData& Frame);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapPinchCall(const FLeapHandData& HandData);

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void LeapUnPinchCall(const FLeapHandData& HandData);

	//UFUNCTION()
	bool GetUseOpenXR();

	//UFUNCTION()
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

	//UPROPERTY()
	bool bUseOpenXR;

	bool bUseDeviceOrigin;

	APawn* LeapPawn;
};
