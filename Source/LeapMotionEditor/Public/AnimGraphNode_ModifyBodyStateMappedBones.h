#pragma once

#include "AnimGraphDefinitions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Editor/AnimGraph/Classes/AnimGraphNode_SkeletalControlBase.h"

#include "AnimNode_ModifyBodyStateMappedBones.h"
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
	// End of UEdGraphNode interface

protected:
	virtual FText GetControllerDescription() const;

};