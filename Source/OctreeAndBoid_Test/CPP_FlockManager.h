// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "CPP_BoidHelper.h"
#include "FSpatialHashGrid.h"
#include "CPP_FlockManager.generated.h"

USTRUCT(BlueprintType)

struct FBoid
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category="Flock Settings")
	FVector Position;

	UPROPERTY(VisibleAnywhere, Category="Flock Settings")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, Category="Flock Settings")
	FVector Acceleration;
	
	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float MinSpeed;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float MaxForce;
	
	
	
	FBoid()
		: Position(FVector::ZeroVector)
		, Velocity(FVector::ZeroVector)
		, Acceleration(FVector::ZeroVector)
		, MaxSpeed(500.f)
		, MinSpeed(300.0f)
		, MaxForce(300.f)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flock Settings")
	FBoid BoidStruct;

	FSpatialHashGrid SpatialHashGrid;
	
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* InstancedMesh;

	
	UPROPERTY(EditAnywhere, Category="Flock Settings")
	int32 NumberOfBoids = 50;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	UStaticMesh* BoidMesh;

	TArray<FBoid> Boids;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	bool bShowDebugAvoidance;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float AlignThreshold = 600.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float CohesionThreshold = 800.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float SeparationThreshold = 300.f;
	
	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float AlignmentFactor;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float CohesionFactor;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float SeparationFactor;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float ObstacleAvoidanceFactor;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float BoundaryRadius = 2000.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float BoundaryAvoidanceFactor = 3.0f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float BoundaryAvoidanceThreshold = 500.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float CollisionAvoidDistance = 500.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	float BoundsRadius = 30.f;

	UPROPERTY(EditAnywhere, Category="Flock Settings")
	TEnumAsByte<ECollisionChannel> ObstacleChannel = ECC_WorldStatic;
	
	void InitializeBoids();
	void UpdateBoids(float DeltaTime);
	void ApplyFlockingForces(FBoid& Boid, int32 BoidIndex, TArray<int32>& NeighbourIndecies);
	bool IsHeadingForCollision(FBoid& Boid);
	FVector SteerTowards(const FVector& DesiredDirection, FBoid& Boid);
	FVector ObstacleRays(FBoid& Boid);
	FVector Align(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex);
	FVector Cohesion(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex);
	FVector Separation(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex);
	FVector AvoidBoundary(FBoid& Boid);
	
};
