// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BoidActor.h"

#include "Components/SphereComponent.h"

// Sets default values
ACPP_BoidActor::ACPP_BoidActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	PerecptionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	PerecptionSphere->SetupAttachment(RootComponent);

	Velocity = FVector3d(0, 0, 0);

	//PerecptionSphere->OnComponentBeginOverlap.AddDynamic(this,&ACPP_BoidActor::OnBeginOverlap);
	//PerecptionSphere->OnComponentEndOverlap.AddDynamic(this,&ACPP_BoidActor::OnEndOverlap);

	SeparationDistance = 50.0f;

}

// Called when the game starts or when spawned
void ACPP_BoidActor::BeginPlay()
{
	Super::BeginPlay();
	
	Neighbours.Empty();

	TSubclassOf<AActor> ClassFilter = ACPP_BoidActor::StaticClass();

	TArray<AActor*> Actors;
	
	//PerecptionSphere->GetOverlappingActors(Actors,ClassFilter);

	// for (AActor* Actor : Actors)
	// {
	// 	if (ACPP_BoidActor* boid = Cast<ACPP_BoidActor>(Actor))
	// 	{
	// 		Neighbours.Add(boid);
	// 	}
	// }
}

// Called every frame
void ACPP_BoidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACPP_BoidActor::UpdateBoid(const FVector& NewVelocity, float DeltaTime)
{

	Velocity = NewVelocity;
	
	FVector3d NewLocation = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLocation);

	if (!Velocity.IsNearlyZero())
	{
		FRotator NewRotation = Velocity.Rotation();
		SetActorRotation(NewRotation);
	}
}

/*
TArray<ACPP_BoidActor*> ACPP_BoidActor::GetNeighbours()
{
	return Neighbours;
}

void ACPP_BoidActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACPP_BoidActor* OtherBoid = Cast<ACPP_BoidActor>(OtherActor))
	{
		if (OtherBoid != this)
		{
			Neighbours.AddUnique(OtherBoid);
		}
	}
}

void ACPP_BoidActor::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACPP_BoidActor* OtherBoid = Cast<ACPP_BoidActor>(OtherActor))
	{
		Neighbours.Remove(OtherBoid);
	}
}
*/

//Empties neighbour list and repopulates it
/*void ACPP_BoidActor::CleanupNeighbours()
{
	Neighbours.Empty();
	TArray<AActor*> OverlappingActors;
	PerecptionSphere->GetOverlappingActors(OverlappingActors, ACPP_BoidActor::StaticClass());

	Neighbours.Reserve(OverlappingActors.Num());
	
	for (int i = 0; i < OverlappingActors.Num(); i++)
	{
		if (OverlappingActors[i] == this) continue;
		
		if (IsValid(OverlappingActors[i]))
		{
			Neighbours.Add(Cast<ACPP_BoidActor>(OverlappingActors[i]));
		}
	}
}*/


