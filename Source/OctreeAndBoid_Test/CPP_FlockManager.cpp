// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_FlockManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "MeshPaintVisualize.h"
#include "Misc/TypeContainer.h"

// Sets default values
ACPP_FlockManager::ACPP_FlockManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	SpatialHashGrid = FSpatialHashGrid(300, true);

	InitializeBoids();

	GetWorld()->GetTimerManager().SetTimer(
		BoidUpdateTimerHandle,
		this,
		&ACPP_FlockManager::UpdateBoids,
		BoidUpdateInterval,
		true
	);
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called. BoidUpdateInterval = %f"), BoidUpdateInterval);
}

void ACPP_FlockManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BoidUpdateTimerHandle);
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ACPP_FlockManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	InterpElapsed += DeltaTime;
	float Alpha = FMath::Clamp(InterpElapsed / InterpDuration, 0.f, 1.f);
	
	//UE_LOG(LogTemp, Warning, TEXT("Interp Alpha = %f"), Alpha);

	for(int32 i = 0; i < Boids.Num(); ++i)
	{
		const FBoid& Boid = Boids[i];
		FVector InterpPos = FMath::Lerp(Boid.PrevPosition, Boid.TargetPosition, Alpha);
		//FQuat InterpRot = FMath::Lerp(Boid.PrevRotation, Boid.TargetRotation, Alpha);
	 
		FTransform NewTransform;
		NewTransform.SetLocation(InterpPos);
		NewTransform.SetRotation(Boid.TargetRotation);
	 
		InstancedMesh->UpdateInstanceTransform(i, NewTransform, true, true);
	}
	 
	InstancedMesh->MarkRenderStateDirty();
	
}

void ACPP_FlockManager::InitializeBoids()
{
	InterpElapsed = 0.f;
	InterpDuration = BoidUpdateInterval;
	
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
		
		Boids[i].PrevPosition = Boids[i].Position;
		Boids[i].TargetPosition = Boids[i].Position;

		Boids[i].PrevRotation = FQuat(Boids[i].Velocity.Rotation());
		Boids[i].TargetRotation = FQuat(Boids[i].Velocity.Rotation());
	}
}

void ACPP_FlockManager::UpdateBoids()
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateBoids triggered"));
	//Clears and repopulates the spatial hash grid each frame
	SpatialHashGrid.ClearGrid();

	for (int32 i = 0; i < Boids.Num(); i++)
	{
		SpatialHashGrid.InsertBoid(i,Boids[i].Position);
	}

	//Draws the active grid cells
	if (bDrawGrid)
	{
		SpatialHashGrid.DrawGrid(GetWorld(), FColor::Red, 0.0f);
	}
	
	for (int32 i = 0; i < Boids.Num(); i++)
	{
		TArray<int32> NeighbourIndecies;

		SpatialHashGrid.GetNeighbourBoids(Boids[i].Position, NeighbourIndecies);
		
		FBoid& Boid = Boids[i];

		Boid.PrevPosition = Boid.Position;
		Boid.PrevRotation = FQuat(Boid.Velocity.Rotation());

		Boid.MaxSpeed = BoidStruct.MaxSpeed;
		Boid.MinSpeed = BoidStruct.MinSpeed;
		Boid.MaxForce = BoidStruct.MaxForce;

		ApplyFlockingForces(Boid, i, NeighbourIndecies);

		if (IsHeadingForCollision(Boid))
		{
			FVector AvoidDir = SteerTowards(ObstacleRays(Boid),Boid) * ObstacleAvoidanceFactor ;
			
			Boid.Acceleration += AvoidDir;
		}
		
		FVector DesiredAcceleration = Boid.Acceleration.GetClampedToMaxSize(Boid.MaxForce);
		Boid.Acceleration = FMath::Lerp(Boid.Acceleration, DesiredAcceleration, 0.1f);
		
		//Boid.Velocity += Boid.Acceleration * BoidUpdateInterval;
;
		FVector NewVelocity = Boid.Velocity + Boid.Acceleration * BoidUpdateInterval;
		Boid.Velocity = FMath::Lerp(Boid.Velocity, NewVelocity, 0.2f);
		
		Boid.Velocity = Boid.Velocity.GetClampedToMaxSize(Boid.MaxSpeed);
		Boid.Velocity = Boid.Velocity.GetClampedToSize(Boid.MinSpeed, Boid.MaxSpeed);
		Boid.Position += Boid.Velocity * BoidUpdateInterval;

		Boid.Acceleration = FVector::ZeroVector;

		Boid.TargetPosition = Boid.Position;
		
		FQuat DesiredRotation = FQuat(Boid.Velocity.Rotation());
		Boid.TargetRotation = FQuat::Slerp(Boid.TargetRotation, DesiredRotation, 0.2f);
	}
	//DrawDebugSphere(GetWorld(), GetActorLocation(), BoundaryRadius, 32, FColor::Red, false, -1.f, 0, 2.f);
	InterpElapsed = 0.f;
	InterpDuration = BoidUpdateInterval;
}

void ACPP_FlockManager::ApplyFlockingForces(FBoid& Boid, int32 BoidIndex, TArray<int32>& NeighbourIndecies)
{
	// Boid.MaxForce = BoidStruct.MaxForce;
	// FVector RandomForce = FMath::VRand() * Boid.MaxForce*0.5;

	//Populates the boid neighbour list with all the relevant boids from the neighbour indecies list
	TArray<FBoid> Neighbours;
	for (int32 i = 0; i < NeighbourIndecies.Num(); i++)
	{
		Neighbours.Add(Boids[NeighbourIndecies[i]]);
	}

	FVector AlignVector = SteerTowards(Align(Neighbours, Boid, BoidIndex), Boid) * AlignmentFactor;
	Boid.Acceleration += AlignVector;

	FVector CohesionVector = SteerTowards(Cohesion(Neighbours, Boid, BoidIndex), Boid) * CohesionFactor;
	Boid.Acceleration += CohesionVector;

	FVector SeparationVector = SteerTowards(Separation(Neighbours, Boid, BoidIndex), Boid) * SeparationFactor;
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

//Starts checking for available directions to steer towards
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
			DrawDebugLine(GetWorld(),Start,End,FColor::Red,false,-1.0f,0,2.0f);
		}
		
		if (!GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ObstacleChannel,FCollisionShape::MakeSphere(BoundsRadius),Params))
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
		if (Distance < AlignThreshold) 
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
		if (Distance < CohesionThreshold)
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
	float DesiredSeparation = SeparationThreshold;

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

