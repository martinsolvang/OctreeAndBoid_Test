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

	Velocity = FVector::ZeroVector;

	PerecptionSphere->OnComponentBeginOverlap.AddDynamic(this,&ACPP_BoidActor::OnBeginOverlap);
	PerecptionSphere->OnComponentEndOverlap.AddDynamic(this,&ACPP_BoidActor::OnEndOverlap);

}

// Called when the game starts or when spawned
void ACPP_BoidActor::BeginPlay()
{
	Super::BeginPlay();
	
	Neighbours.Empty();
	
}

// Called every frame
void ACPP_BoidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACPP_BoidActor::UpdateBoid(const FVector& NewVelocity, float DeltaTime)
{
	Velocity = NewVelocity;

	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLocation);

	if (!Velocity.IsNearlyZero())
	{
		FRotator NewRotation = Velocity.Rotation();
		SetActorRotation(NewRotation);
	}
}

void ACPP_BoidActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent && OtherActor && OtherActor == Cast<ACPP_BoidActor>(OtherActor))
	{
		Neighbours.Add(Cast<ACPP_BoidActor>(OtherActor));
	}
}

void ACPP_BoidActor::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent && OtherActor && OtherActor == Cast<ACPP_BoidActor>(OtherActor))
	{
		Neighbours.Remove(Cast<ACPP_BoidActor>(OtherActor));
	}
}

//Empties neighbour list and repopulates it
void ACPP_BoidActor::CleanupNeighbours()
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
}


