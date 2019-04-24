// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Skeleton/BodyStateArm.h"

UBodyStateFinger::UBodyStateFinger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UBodyStateHand::UBodyStateHand(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UBodyStateFinger* UBodyStateHand::ThumbFinger()
{
	return Fingers[0];
}

UBodyStateFinger* UBodyStateHand::IndexFinger()
{
	return Fingers[1];
}

UBodyStateFinger* UBodyStateHand::MiddleFinger()
{
	return Fingers[2];
}

UBodyStateFinger* UBodyStateHand::RingFinger()
{
	return Fingers[3];
}

UBodyStateFinger* UBodyStateHand::PinkyFinger()
{
	return Fingers[4];
}

UBodyStateArm::UBodyStateArm(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}
