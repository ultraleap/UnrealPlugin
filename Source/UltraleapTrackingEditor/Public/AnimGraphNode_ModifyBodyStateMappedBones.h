/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/
#pragma once

#include "AnimGraphDefinitions.h"
#include "AnimNode_ModifyBodyStateMappedBones.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Editor/AnimGraph/Public/AnimGraphNode_SkeletalControlBase.h"
#else
#include "Editor/AnimGraph/Classes/AnimGraphNode_SkeletalControlBase.h"
#endif
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphNode_ModifyBodyStateMappedBones.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_ModifyBodyStateMappedBones : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_ModifyBodyStateMappedBones Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FString GetNodeCategory() const override;
	virtual FText GetTooltipText() const override;
	// End of UEdGraphNode interface

protected:
	// UAnimGraphNode_SkeletalControlBase protected interface
	virtual FText GetControllerDescription() const;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override
	{
		return &Node;
	}
	// End of UAnimGraphNode_SkeletalControlBase protected interface
};