// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "UltraleapIEFunctionLibrary.h"

void UUltraleapIEFunctionLibrary::SetMaxDepenetration(UPrimitiveComponent* PrimitiveComponent, float MaxDepenetrationVelocity)
{
	if (PrimitiveComponent == nullptr)
	{
		return;
	}
	// PrimitiveComponent->BodyInstance.MaxDepenetrationVelocity = MaxDepenetrationVelocity;
}