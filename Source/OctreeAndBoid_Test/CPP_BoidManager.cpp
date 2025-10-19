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
	MinSpeed = 100.0f;
	
	MoveSpeedFactor = 1.0f;

	VisionRange = 500.0f;
	
	BoxWitdh = 200.0f;
	BoxDepth = 200.0f;
	BoxHeight = 200.0f;

	BoundingBox = CreateDefaultSubobject<UBoxComponent>("BoundingBox");
	RootComponent = BoundingBox;

	BoundingBox->SetCollisionProfileName("OverlapAll");

	BoundingBox->SetBoxExtent(FVector(BoxWitdh, BoxDepth, BoxHeight));

	SeparationFactor = 15;
	CohesionFactor = 1;
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

		FRotator SpawnRotator = FRotator::ZeroRotator;
        
		ACPP_BoidActor* boid = GetWorld()->SpawnActor<ACPP_BoidActor>(BoidActor, SpawnPos, SpawnRotator);
        
		if (boid)
		{
			AllBoids.Add(boid);
			// Set random velocity direction and magnitude
			boid->Velocity = UKismetMathLibrary::RandomUnitVector() * InitialSpeed;
            
			// Set initial rotation to match velocity
			if (!boid->Velocity.IsNearlyZero())
			{
				boid->SetActorRotation(boid->Velocity.Rotation());
			}
		}

		for (ACPP_BoidActor* Bird : AllBoids)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s"), *Bird->GetName())); 
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("AllBoids count: %d"), AllBoids.Num());
}

// Called when the game starts or when spawned
void ACPP_BoidManager::BeginPlay()
{
	Super::BeginPlay();

	SpawnAndRegisterBoids(NumberOfBoidsToSpawn);
	

	//Find all boids and add to array
	TArray<AActor*> actors;
	
	
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


//Returns a steering vector if the boid is too close to any other boid
FVector3d ACPP_BoidManager::CalculateSeparation(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = AllBoids;
    
	FVector3d result = FVector3d::ZeroVector;
    
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			FVector3d toBoid = Neighbours[i]->GetActorLocation() - Boid->GetActorLocation();
			float distance = toBoid.Length();
            
			if (distance > 0 && distance < Boid->SeparationDistance)
			{
				result += toBoid.GetSafeNormal() / distance;
			}
		}
	}
    
	return result;
}

//Returns a seering vector for the boid to align itself with nearby boids
FVector3d ACPP_BoidManager::CalculateAlignment(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = AllBoids;

	if (Neighbours.Num() == 0) return FVector::ZeroVector;
	
	FVector3d avgVelocity = FVector3d::ZeroVector;
	int32 count = 0;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			float distance = FVector3d::Distance(AllBoids[i]->GetActorLocation(), Boid->GetActorLocation());

			if (distance <= VisionRange)
			{
				avgVelocity += Neighbours[i]->Velocity;
				count++;
			}
		}
	}
	if (count == 0) return FVector::ZeroVector;
    
	avgVelocity /= count;
	
	return avgVelocity - Boid->Velocity;
}


//Returns a vector towards a percieved center of mass for the nearby boids
FVector3d ACPP_BoidManager::CalculateCohesion(ACPP_BoidActor* Boid)
{
	TArray<ACPP_BoidActor*> Neighbours = AllBoids;
	
	if (Neighbours.Num() <= 1) return FVector::ZeroVector;

	FVector3d centerOfMass = FVector3d::ZeroVector;
	int32 count = 0;
	
	for (int i = 0; i < Neighbours.Num(); i++)
	{
		if (Neighbours[i] && Neighbours[i] != Boid)
		{
			float distance = FVector3d::Distance(AllBoids[i]->GetActorLocation(), Boid->GetActorLocation());
			
			if (distance <= VisionRange)
			{
				centerOfMass += Neighbours[i]->GetActorLocation();
				count++;
			}
		}
	}	

	centerOfMass /= count;

	return centerOfMass - Boid->GetActorLocation();
}

void ACPP_BoidManager::UpdateBoids(TArray<ACPP_BoidActor*> Boids, float DeltaTime)
{
	for (auto boid : Boids)
	{
		if (boid)
		{
			// Calculate all forces
			FVector3d separation = CalculateSeparation(boid) * SeparationFactor;
			FVector3d alignment = CalculateAlignment(boid) * AlignmentFactor;
			FVector3d cohesion = CalculateCohesion(boid) * CohesionFactor;
            
			FVector3d steeringForce = separation + alignment + cohesion;
            
			// Apply steering force smoothly
			FVector3d newVelocity = boid->Velocity + steeringForce * DeltaTime;
            
			// Get current speed
			float currentSpeed = newVelocity.Length();
            
			// Clamp speed between min and max
			if (currentSpeed > MaxSpeed)
			{
				newVelocity = newVelocity.GetSafeNormal() * MaxSpeed;
			}
			else if (currentSpeed < MinSpeed && currentSpeed > 0)
			{
				newVelocity = newVelocity.GetSafeNormal() * MinSpeed;
			}
            
			ContainBoids(boid);
            
			// MoveSpeedFactor only affects display speed, not actual velocity
			boid->UpdateBoid(newVelocity, DeltaTime * MoveSpeedFactor);
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

