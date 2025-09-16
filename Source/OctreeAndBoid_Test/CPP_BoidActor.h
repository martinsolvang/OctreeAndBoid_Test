// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_BoidActor.generated.h"

class USphereComponent;

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_BoidActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_BoidActor();
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(EditAnywhere)
	USphereComponent* PerecptionSphere;

	UPROPERTY(EditAnywhere)
	float SeparationDistance;

	FVector Velocity;

	//List of nearby boids, updates with overlap events
	//Used by BoidManager to calculate the Boid algorithm
	TArray<ACPP_BoidActor*> Neighbours;

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Called in BoidManager, updates position and rotation each tick
	void UpdateBoid(const FVector& NewVelocity, float DeltaTime);

	TArray<ACPP_BoidActor*> GetNeighbours();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//Clears and repopulates the neighbour list
	UFUNCTION()
	void CleanupNeighbours();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	

};
