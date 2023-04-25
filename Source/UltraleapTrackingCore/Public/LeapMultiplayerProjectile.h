// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LeapMultiplayerProjectile.generated.h"

class UParticleSystem;
class UStaticMeshComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UDamageType;

UCLASS()
class ULTRALEAPTRACKING_API ALeapMultiplayerProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALeapMultiplayerProjectile();

	// Basic components for this projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	// The damage type and damage that will be done by this projectile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float Damage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	void Destroyed() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};