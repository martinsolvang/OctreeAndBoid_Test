// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BoidManager.h"

#include "CPP_BoidActor.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ACPP_BoidManager::ACPP_BoidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bMoveBoids = true;

	MoveSpeed = 1.0f;

	BoxWitdh = 200.0f;
	BoxDepth = 200.0f;
	BoxHeight = 200.0f;

	BoundingBox = CreateDefaultSubobject<UBoxComponent>("BoundingBox");
	RootComponent = BoundingBox;

	BoundingBox->SetCollisionProfileName("OverlapAll");

	BoundingBox->SetBoxExtent(FVector(BoxWitdh, BoxDepth, BoxHeight));
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

// Called when the game starts or when spawned
void ACPP_BoidManager::BeginPlay()
{
	Super::BeginPlay();

	

	//Find all boids and add to array
	TArray<AActor*> actors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ACPP_BoidActor::StaticClass(),actors);

	for (auto actor : actors)
	{
		if (ACPP_BoidActor* boid = Cast<ACPP_BoidActor>(actor))
		{
			AllBoids.Add(boid);
		}
	}
	
	
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

void ACPP_BoidManager::CalculateSeparation(TArray<ACPP_BoidActor*> Boids)
{
}

void ACPP_BoidManager::CalculateAlignment(TArray<ACPP_BoidActor*> Boids)
{
}

void ACPP_BoidManager::CalculateCohesion(TArray<ACPP_BoidActor*> Boids)
{
}

void ACPP_BoidManager::UpdateBoids(TArray<ACPP_BoidActor*> Boids, float DeltaTime)
{
	for (auto boid : Boids)
	{
		if (boid)
		{
			ContainBoids(boid);
			
			boid->UpdateBoid(boid->GetActorForwardVector().GetSafeNormal()*MoveSpeed,DeltaTime);
		}
	}
}

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

