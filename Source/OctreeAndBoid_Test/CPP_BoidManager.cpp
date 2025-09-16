// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BoidManager.h"

#include "CPP_BoidActor.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ACPP_BoidManager::ACPP_BoidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	NumberOfBoidsToSpawn = 20;

	bMoveBoids = true;

	InitialSpeed = 100.0f;

	MaxSpeed = 400.0f;
	
	MoveSpeedFactor = 1.0f;

	BoxWitdh = 200.0f;
	BoxDepth = 200.0f;
	BoxHeight = 200.0f;

	BoundingBox = CreateDefaultSubobject<UBoxComponent>("BoundingBox");
	RootComponent = BoundingBox;

	BoundingBox->SetCollisionProfileName("OverlapAll");

	BoundingBox->SetBoxExtent(FVector(BoxWitdh, BoxDepth, BoxHeight));

	SeparationFactor = 1;
	CohesionFactor = 0.05;
	AlignmentFactor = 1;
	
}
void ACPP_BoidManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (BoundingBox)
	{
		BoundingBox->SetBoxExtent(FVector(BoxWitdh, BoxDepth, BoxHeight));
	}
#if WITH_EDITOR
	// Draw a wireframe box in the editor
	DrawDebugBox(
		GetWorld(),
		BoundingBox->GetComponentLocation(),
		BoundingBox->GetScaledBoxExtent(),
		FColor::Green,
		false,  // Not persistent (so it refreshes when you move it)
		-1.0f,  // Duration
		0,
		2.0f    // Thickness
	);
#endif
}

void ACPP_BoidManager::SpawnAndRegisterBoids(int32 SpawnNumber)
{
	FVector BoxCenter = BoundingBox->GetComponentLocation();
	
	for (int i = 0; i < NumberOfBoidsToSpawn; i++)
	{
		float RandX = UKismetMathLibrary::RandomFloatInRange(BoxCenter.X - BoxWitdh, BoxCenter.X + BoxWitdh);
		float RandY	= UKismetMathLibrary::RandomFloatInRange(BoxCenter.Y - BoxDepth, BoxCenter.Y + BoxDepth);
		float RandZ	= UKismetMathLibrary::RandomFloatInRange(BoxCenter.Z - BoxHeight, BoxCenter.Z + BoxHeight);
			
		FVector SpawnPos = FVector(RandX,RandY,RandZ);

		FRotator SpawnRotator = FRotator(UKismetMathLibrary::RandomFloatInRange(-20,20),
				UKismetMathLibrary::RandomFloatInRange(-20,20),
				 UKismetMathLibrary::RandomFloatInRange(0,360));
		
		ACPP_BoidActor* boid = GetWorld()->SpawnActor<ACPP_BoidActor>(BoidActor,SpawnPos,SpawnRotator);
			
		if (boid)
		{
			AllBoids.Add(boid);
			boid->Velocity = UKismetMathLibrary::RandomUnitVector() * InitialSpeed;
		}
	}
}

// Called when the game starts or when spawned
void ACPP_BoidManager::BeginPlay()
{
	Super::BeginPlay();

	SpawnAndRegisterBoids(NumberOfBoidsToSpawn);
	

	//Find all boids and add to array
	//TArray<AActor*> actors;

	// UGameplayStatics::GetAllActorsOfClass(GetWorld(),ACPP_BoidActor::StaticClass(),actors);
	//
	// for (auto actor : actors)
	// {
	// 	if (ACPP_BoidActor* boid = Cast<ACPP_BoidActor>(actor))
	// 	{
	// 		AllBoids.Add(boid);
	// 	}
	// }
	
	
}

// Called every frame
void ACPP_BoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bMoveBoids && AllBoids.Num() > 0)
	{
		UpdateBoids(AllBoids, DeltaTime);
	}
}

//Results a steering vector if the boid is too close to any other boid, within a certain radius
FVector3d ACPP_BoidManager::CalculateSeparation(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = Boid->GetNeighbours();

	FVector3d result = FVector3d(0.0f, 0.0f, 0.0f); 
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			FVector3d BoidPos = Boid->GetActorLocation();
			FVector3d NeighbourPos = Neighbours[i]->GetActorLocation();
			FVector3d DistVector = FVector3d(NeighbourPos - BoidPos);

			if (DistVector.Length() < Boid->SeparationDistance)
			{
				result -= DistVector;
			}
		}
	}

	return result;
	
}

//Returns a seering vector for the boid to align itself with nearby boids
FVector3d ACPP_BoidManager::CalculateAlignment(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = Boid->GetNeighbours();

	FVector3d result = FVector3d(0.0f, 0.0f, 0.0f);

	if (Neighbours.Num() == 0) return FVector::ZeroVector;
	
	float Vx = 0;
	float Vy = 0;
	float Vz = 0;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			Vx += Neighbours[i]->GetActorForwardVector().X;
			Vy += Neighbours[i]->GetActorForwardVector().Y;
			Vz += Neighbours[i]->GetActorForwardVector().Z;
		}
	}

	Vx = Vx / Neighbours.Num();
	Vy = Vy / Neighbours.Num();
	Vz = Vz / Neighbours.Num();

	result = FVector3d(Vx, Vy, Vz);
	
	return result;
}


//Returns a vector towards a percieved center of mass for the nearby boids
FVector3d ACPP_BoidManager::CalculateCohesion(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = Boid->GetNeighbours();

	FVector3d result = FVector3d(0.0f, 0.0f, 0.0f); 

	float Vx = 0;
	float Vy = 0;
	float Vz = 0;

	if (Neighbours.Num() == 0) return FVector::ZeroVector;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			Vx += (Neighbours[i]->GetActorLocation().X - Boid->GetActorLocation().X);
			Vy += (Neighbours[i]->GetActorLocation().Y - Boid->GetActorLocation().Y);
			Vz += (Neighbours[i]->GetActorLocation().Z - Boid->GetActorLocation().Z);
		}
	}
	
	Vx = Vx / Neighbours.Num();
	Vy = Vy / Neighbours.Num();
	Vz = Vz / Neighbours.Num();

	result = FVector3d(Vx, Vy, Vz);

	return result;
}

void ACPP_BoidManager::UpdateBoids(TArray<ACPP_BoidActor*> Boids, float DeltaTime)
{
	for (auto boid : Boids)
	{
		if (boid)
		{
			FVector3d SteeringVector = FVector3d(0.0f, 0.0f, 0.0f);
			FVector3d FinalVector = boid->Velocity;

			SteeringVector += CalculateSeparation(boid)*SeparationFactor + CalculateAlignment(boid)*AlignmentFactor + CalculateCohesion(boid)*CohesionFactor;
			
			FinalVector += SteeringVector*MoveSpeedFactor;

			FinalVector = FinalVector.GetClampedToMaxSize(MaxSpeed);
			
			ContainBoids(boid);

			boid->UpdateBoid(FinalVector,DeltaTime);
		}
	}
}


//THIS MESSES WITH NEIGHBOUR LIST NEED TO FIX
void ACPP_BoidManager::ContainBoids(ACPP_BoidActor* Boid)
{
	bool bContained = false;
	
	FVector BoidLocation = Boid->GetActorLocation();
	FVector BoxCenter = BoundingBox->GetComponentLocation();

	//Checks X axis
	if (BoidLocation.X > (BoxCenter.X + BoxWitdh))
	{
		BoidLocation.X = (BoxCenter.X - BoxWitdh);
		bContained = true;
	}
	else if (BoidLocation.X < (BoxCenter.X - BoxWitdh))
	{
		BoidLocation.X = (BoxCenter.X + BoxWitdh);
		bContained = true;
	}

	//Checks Y axis
	if (BoidLocation.Y > (BoxCenter.Y + BoxDepth))
	{
		BoidLocation.Y = (BoxCenter.Y - BoxDepth);
		bContained = true;
	}
	else if (BoidLocation.Y < (BoxCenter.Y - BoxDepth))
	{
		BoidLocation.Y = (BoxCenter.Y + BoxDepth);
		bContained = true;
	}

	//Checks Z axis
	if (BoidLocation.Z > (BoxCenter.Z + BoxHeight))
	{
		BoidLocation.Z = (BoxCenter.Z - BoxHeight);
		bContained = true;
	}
	else if (BoidLocation.Z < (BoxCenter.Z - BoxHeight))
	{
		BoidLocation.Z = (BoxCenter.Z + BoxHeight);
		bContained = true;
	}

	if (bContained)
	{
		Boid->SetActorLocation(BoidLocation);
	}
}

