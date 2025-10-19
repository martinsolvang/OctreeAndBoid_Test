// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_FlockManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values
ACPP_FlockManager::ACPP_FlockManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bCanEverTick = true;

	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	RootComponent = InstancedMesh;
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
		Boids[i].Velocity = FMath::VRand() * 200.f;

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
		InstancedMesh->UpdateInstanceTransform(i, NewTransform, true, true);
	}

	InstancedMesh->MarkRenderStateDirty();
}

void ACPP_FlockManager::ApplyFlockingForces(FBoid& Boid, int32 BoidIndex)
{
	FVector RandomForce = FMath::VRand() * Boid.MaxForce;
	Boid.Acceleration += RandomForce;
}
