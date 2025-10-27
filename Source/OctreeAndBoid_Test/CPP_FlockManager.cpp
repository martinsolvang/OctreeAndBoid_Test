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
	ObstacleAvoidanceFactor = 10.0f;

	bShowDebugAvoidance = false;

}

// Called when the game starts or when spawned
void ACPP_FlockManager::BeginPlay()
{
	Super::BeginPlay();

	if (BoidMesh)
	{
		InstancedMesh->SetStaticMesh(BoidMesh);
	}
	
	FCPP_BoidHelper::Init();

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

		Boids[i] = BoidStruct;

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

		Boid.MaxSpeed = BoidStruct.MaxSpeed;
		Boid.MinSpeed = BoidStruct.MinSpeed;
		Boid.MaxForce = BoidStruct.MaxForce;

		ApplyFlockingForces(Boid, i);

		if (IsHeadingForCollision(Boid))
		{
			FVector AvoidDir = SteerTowards(ObstacleRays(Boid),Boid) * ObstacleAvoidanceFactor ;
			
			Boid.Acceleration += AvoidDir;
		}
		
		FVector DesiredAcceleration = Boid.Acceleration.GetClampedToMaxSize(Boid.MaxForce);
		Boid.Acceleration = FMath::Lerp(Boid.Acceleration, DesiredAcceleration, 0.1f);
		
		Boid.Velocity += Boid.Acceleration * DeltaTime;
		Boid.Velocity = Boid.Velocity.GetClampedToMaxSize(Boid.MaxSpeed);
		Boid.Velocity = Boid.Velocity.GetClampedToSize(Boid.MinSpeed, Boid.MaxSpeed);
		Boid.Position += Boid.Velocity * DeltaTime;

		Boid.Acceleration = FVector::ZeroVector;

		FTransform NewTransform;
		
		NewTransform.SetLocation(Boid.Position);
        NewTransform.SetRotation(FQuat(Boid.Velocity.Rotation()));
		InstancedMesh->UpdateInstanceTransform(i, NewTransform, true, true);
		
	}

	InstancedMesh->MarkRenderStateDirty();
	
	//DrawDebugSphere(GetWorld(), GetActorLocation(), BoundaryRadius, 32, FColor::Red, false, -1.f, 0, 2.f);
}

void ACPP_FlockManager::ApplyFlockingForces(FBoid& Boid, int32 BoidIndex)
{
	// Boid.MaxForce = BoidStruct.MaxForce;
	// FVector RandomForce = FMath::VRand() * Boid.MaxForce*0.5;

	FVector AlignVector = SteerTowards(Align(Boids, Boid, BoidIndex), Boid) * AlignmentFactor;
	Boid.Acceleration += AlignVector;

	FVector CohesionVector = SteerTowards(Cohesion(Boids, Boid, BoidIndex), Boid) * CohesionFactor;
	Boid.Acceleration += CohesionVector;

	FVector SeparationVector = SteerTowards(Separation(Boids, Boid, BoidIndex), Boid) * SeparationFactor;
	Boid.Acceleration += SeparationVector;
	
	Boid.Acceleration += AvoidBoundary(Boid);
	//Boid.Acceleration += RandomForce;
}

bool ACPP_FlockManager::IsHeadingForCollision(FBoid& Boid)
{
	FHitResult Hit;
	FVector Start = Boid.Position;
	FVector End = Start + Boid.Velocity.GetSafeNormal() * CollisionAvoidDistance;

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	return GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ObstacleChannel,
		FCollisionShape::MakeSphere(BoundsRadius),
		Params
	);
}

FVector ACPP_FlockManager::SteerTowards(const FVector& DesiredDirection, FBoid& Boid)
{
	
	FVector DesiredVelocity = DesiredDirection.GetSafeNormal() * Boid.MaxSpeed;
	FVector Steering = DesiredVelocity - Boid.Velocity;
	return Steering.GetClampedToMaxSize(Boid.MaxForce);
}

FVector ACPP_FlockManager::ObstacleRays(FBoid& Boid)
{
	TArray<FVector> RayDirections = FCPP_BoidHelper::Directions;

	FRotator Rotation = Boid.Velocity.Rotation();
	
	for (int32 i = 0; i < RayDirections.Num(); i++)
	{

		FVector Dir = Rotation.RotateVector(RayDirections[i]);
		Dir.Normalize();
		
		FVector Start = Boid.Position;
		FVector End = Start + Dir * CollisionAvoidDistance;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.bTraceComplex = false;
		Params.AddIgnoredActor(this);
		
		//Debug avoidance
		if (bShowDebugAvoidance)
		{
			DrawDebugLine(
			   GetWorld(),
			   Start,
			   End,
			   FColor::Red,
			   false,
			   -1.0f,
			   0,
			   2.0f
			);

		}
		
		if (!GetWorld()->SweepSingleByChannel(
			Hit,
			Start,
			End,
			FQuat::Identity,
			ObstacleChannel,
			FCollisionShape::MakeSphere(BoundsRadius),
			Params))
		{
			return Dir;
		}
	}
	
	return Boid.Velocity;
}

FVector ACPP_FlockManager::Align(const TArray<FBoid>& Neighbours, const FBoid& Boid, int32 BoidIndex)
{
	if (Neighbours.Num() < 0)
	{
		return FVector::ZeroVector;
	}
	FVector AvgVelocity = FVector::ZeroVector;
	int32 Count = 0;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (i == BoidIndex)
			continue;

		float Distance = FVector::Dist(Neighbours[i].Position, Boid.Position);
		if (Distance < 600.f) 
		{
			AvgVelocity += Neighbours[i].Velocity;
			Count++;
		}
	}

	if (Count > 0)
	{
		AvgVelocity /= Count;
                     
		return AvgVelocity;
	}

	return FVector::ZeroVector;
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
		CenterOfMass /= Count;
		return  CenterOfMass - Boid.Position;
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
		Steering /= Count;
	}

	if (!Steering.IsNearlyZero())
	{
		return Steering;
	}

	return FVector::ZeroVector;
}


//JUST FOR TESTING
FVector ACPP_FlockManager::AvoidBoundary(FBoid& Boid)
{

	FVector CenterPoint = GetActorLocation(); 
	FVector ToBoid = Boid.Position - CenterPoint;
	float Distance = ToBoid.Size();
    
	
	if (Distance > BoundaryRadius - BoundaryAvoidanceThreshold)
	{
		
		float Strength = FMath::Max(Distance - (BoundaryRadius - BoundaryAvoidanceThreshold), 0.f);
		FVector Steering = (-ToBoid.GetSafeNormal() * Boid.MaxSpeed) - Boid.Velocity;
		Steering = Steering.GetClampedToMaxSize(Boid.MaxForce);
		return Steering * BoundaryAvoidanceFactor * (Strength / BoundaryAvoidanceThreshold);
	}
    
	return FVector::ZeroVector;
}

