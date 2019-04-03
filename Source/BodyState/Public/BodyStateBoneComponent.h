// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "BodyStateBoneComponent.generated.h"

/** 
* Scene Component which syncs to a desired BodyStateBone
*/
UCLASS(ClassGroup = "BodyState", meta = (BlueprintSpawnableComponent))
class BODYSTATE_API UBodyStateBoneComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body State Bone Component")
	int32 SkeletonId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Body State Bone Component")
	EBodyStateBasicBoneType BoneToFollow;

protected:

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
};