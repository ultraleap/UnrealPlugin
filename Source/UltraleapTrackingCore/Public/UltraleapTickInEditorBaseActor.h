/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UltraleapTickInEditorBaseActor.generated.h"

UCLASS()
class ULTRALEAPTRACKING_API AUltraleapTickInEditorBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUltraleapTickInEditorBaseActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// allow tick in editor
	virtual bool ShouldTickIfViewportsOnly() const override;

	/** Tick that runs ONLY in the editor viewport.*/
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Ultraleap Events")
	void EditorTick(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Ultraleap Events")
	bool bTickInEditor;
};
