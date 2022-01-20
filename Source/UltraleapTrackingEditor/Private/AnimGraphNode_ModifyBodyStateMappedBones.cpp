/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "AnimGraphNode_ModifyBodyStateMappedBones.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_ModifyBSHand

UAnimGraphNode_ModifyBodyStateMappedBones::UAnimGraphNode_ModifyBodyStateMappedBones(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

// Title Color!
FLinearColor UAnimGraphNode_ModifyBodyStateMappedBones::GetNodeTitleColor() const
{
	return FLinearColor(12, 12, 0, 1);
}

// Node Category
FString UAnimGraphNode_ModifyBodyStateMappedBones::GetNodeCategory() const
{
	return FString("Body State Animation (Ultraleap)");
}
FText UAnimGraphNode_ModifyBodyStateMappedBones::GetControllerDescription() const
{
	return FText::FromString("Ultraleap Modify Mapped Bones (Body State)");
}

FText UAnimGraphNode_ModifyBodyStateMappedBones::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText Result = GetControllerDescription();
	return Result;
}
FText UAnimGraphNode_ModifyBodyStateMappedBones::GetTooltipText() const
{
	return FText::FromString(
		"Animates the skeletel mesh using the Ultraleap tracked device hand and optionally auto maps to the skeleton");
}