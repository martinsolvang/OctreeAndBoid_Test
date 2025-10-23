// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_FlockManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Misc/TypeContainer.h"

// Sets default values
ACPP_FlockManager::ACPP_FlockManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bCanEverTick = true;

	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	RootComponent = InstancedMesh;

	InstancedMesh->bUseAsOccluder = false;
	InstancedMesh->InstanceStartCullDistance = 100.f;
	InstancedMesh->InstanceEndCullDistance = 10000.f;

	AlignmentFactor = 2.0f;
	CohesionFactor = 2.0f;
	SeparationFactor = 1.0f;
}

// Called when the game starts or when spawned
void ACPP_FlockManager::BeginPlay()
{
	Super::BeginPlay();

	if (BoidMesh)
	{
		InstancedMesh->SetStaticMesh(BoidMesh);
	}

	InitializeBoids();
}

// Called every frame
void ACPP_FlockManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateBoids(DeltaTime);
}

void ACPP_FlockManager::InitializeBoids()
{
	Boids.SetNum(NumberOfBoids);

	for (int32 i = 0; i < NumberOfBoids; i++)
	{
		FVector SpawnLocation = GetActorLocation() + FMath::VRand() * 500.f;

		Boids[i].Position = SpawnLocation;
		Boids[i].Velocity = FMath::VRand() * 500.f;

		FTransform Transform;
		Transform.SetLocation(SpawnLocation);
		InstancedMesh->AddInstance(Transform);
	}
}

void ACPP_FlockManager::UpdateBoids(float DeltaTime)
{
	for (int32 i = 0; i < Boids.Num(); i++)
	{
		FBoid& Boid = Boids[i];

		ApplyFlockingForces(Boid, i);

		Boid.Velocity += Boid.Acceleration * DeltaTime;
		Boid.Velocity = Boid.Velocity.GetClampedToMaxSize(Boid.MaxSpeed);
		Boid.Position += Boid.Velocity * DeltaTime;

		Boid.Acceleration = FVector::ZeroVector;

		FTransform NewTransform;
		
		NewTransform.SetLocation(Boid.Position);
        NewTransform.SetRotation(FQuat(Boid.Velocity.Rotation()));
		InstancedMesh->UpdateInstanceTransform(i, NewTransform, true, true);
		
	}

	InstancedMesh->MarkRenderStateDirty();
}

void ACPP_FlockManager::ApplyFlockingForces(FBoid& Boid, int32 BoidIndex)
{
	//FVector RandomForce = FMath::VRand() * Boid.MaxForce;
	Boid.Acceleration += Align(Boids, Boid, BoidIndex);
	Boid.Acceleration += Cohesion(Boids, Boid, BoidIndex);
	Boid.Acceleration += Separation(Boids, Boid, BoidIndex);
	//Boid.Acceleration += RandomForce;
}

FVector ACPP_FlockManager::Align(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex)
{
	if (Neighbours.Num() < 0)
	{
		return FVector::ZeroVector;
	}
	FVector Steering = FVector::ZeroVector;
	int32 Count = 0;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (i == BoidIndex)
			continue;

		float Distance = FVector::Dist(Neighbours[i].Position, Boid.Position);
		if (Distance < 600.f) 
		{
			Steering += Neighbours[i].Velocity;
			Count++;
		}
	}

	if (Count > 0)
	{
		Steering /= (float)Count;                       
		Steering = Steering.GetSafeNormal() * Boid.MaxSpeed; 
		Steering -= Boid.Velocity;                      
		Steering = Steering.GetClampedToMaxSize(Boid.MaxForce);
	}

	return Steering * AlignmentFactor;
}

FVector ACPP_FlockManager::Cohesion(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex)
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 i = 0; i < Neighbours.Num(); i++)
	{
		if (i == BoidIndex) continue;

		float Distance = FVector::Dist(Neighbours[i].Position, Boid.Position);
		if (Distance < 800.f)
		{
			CenterOfMass += Neighbours[i].Position;
			Count++;
		}
	}

	if (Count > 0)
	{
		CenterOfMass /= (float)Count;
		FVector Desired = (CenterOfMass - Boid.Position).GetSafeNormal() * Boid.MaxSpeed;
		FVector Steering = (Desired - Boid.Velocity).GetClampedToMaxSize(Boid.MaxForce);
		return Steering * CohesionFactor;
	}

	return FVector::ZeroVector;
}

FVector ACPP_FlockManager::Separation(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex)
{
	FVector Steering = FVector::ZeroVector;
	int32 Count = 0;
	float DesiredSeparation = 300.f;

	for (int32 i = 0; i < Neighbours.Num(); i++)
	{
		if (i == BoidIndex) continue;

		float Distance = FVector::Dist(Neighbours[i].Position, Boid.Position);
		if (Distance > 0 && Distance < DesiredSeparation)
		{
			FVector Diff = (Boid.Position - Neighbours[i].Position).GetSafeNormal();
			Diff /= Distance; 
			Steering += Diff;
			Count++;
		}
	}

	if (Count > 0)
	{
		Steering /= (float)Count;
	}

	if (!Steering.IsNearlyZero())
	{
		Steering = Steering.GetSafeNormal() * Boid.MaxSpeed;
		Steering -= Boid.Velocity;
		Steering = Steering.GetClampedToMaxSize(Boid.MaxForce);
	}

	return Steering * SeparationFactor;
}

