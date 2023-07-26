// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LeapSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ULTRALEAPTRACKING_API ULeapSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	ULeapSaveGame();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	TMap<FString, FPoseSnapshot*> Poses;

};
