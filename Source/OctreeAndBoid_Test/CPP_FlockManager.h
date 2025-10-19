// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "CPP_FlockManager.generated.h"

USTRUCT()

struct FBoid
{
	GENERATED_BODY()

	FVector Position;
	FVector Velocity;
	FVector Acceleration;
	float MaxSpeed;
	float MaxForce;

	FBoid()
		: Position(FVector::ZeroVector)
		, Velocity(FVector::ZeroVector)
		, Acceleration(FVector::ZeroVector)
		, MaxSpeed(400.f)
		, MaxForce(50.f)
	{}
};

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_FlockManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACPP_FlockManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* InstancedMesh;

	
	UPROPERTY(EditAnywhere, Category="Flock Settings")
	int32 NumberOfBoids = 50;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	UStaticMesh* BoidMesh;

	TArray<FBoid> Boids;
	
	void InitializeBoids();
	void UpdateBoids(float DeltaTime);
	void ApplyFlockingForces(FBoid& Boid, int32 BoidIndex);
};
