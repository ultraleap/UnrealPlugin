/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "UltraleapEditorNotifyComponent2.generated.h"
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ULTRALEAPTRACKING_API UUltraleapEditorNotifyComponent2 : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	/** Native callback when this component is moved */
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	// called after the component is moved in the editor, override in blueprint
	UFUNCTION(BlueprintImplementableEvent)
	void HandleComponentMove(const bool bFinished);
};
