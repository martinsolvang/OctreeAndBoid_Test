// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPinComboBox.h"
#include "GameFramework/Actor.h"
#include "CPP_BoidManager.generated.h"

class ACPP_BoidActor;
class UBoxComponent;

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_BoidManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACPP_BoidManager();

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACPP_BoidActor> BoidActor;

	UPROPERTY(EditAnywhere)
	int32 NumberOfBoidsToSpawn;
	
	UPROPERTY(EditAnywhere)
	bool bMoveBoids;

	UPROPERTY(EditAnywhere)
	float MoveSpeedFactor;

	UPROPERTY(EditAnywhere)
	float BoxWitdh;

	UPROPERTY(EditAnywhere)
	float BoxHeight;

	UPROPERTY(EditAnywhere)
	float BoxDepth;

	UPROPERTY(EditAnywhere)
	UBoxComponent* BoundingBox;
	
private:
	UPROPERTY(EditAnywhere)
	float SeparationFactor;

	UPROPERTY(EditAnywhere)
	float AlignmentFactor;

	UPROPERTY(EditAnywhere)
	float CohesionFactor;

	//temporary list for calculation
	//NOT IN USE YET, Only if sphere overlap does not work
	
	UPROPERTY(VisibleAnywhere)
	TArray<ACPP_BoidActor*> AllBoids;

	UPROPERTY(EditAnywhere)
	float BoidVisionRange;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;

	UPROPERTY(EditAnywhere)
	float InitialSpeed;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;


	//Planned method to spawn boids within a set box bound
	void SpawnAndRegisterBoids( int32 SpawnNumber);
	
	FVector3d CalculateSeparation(ACPP_BoidActor* Boid);

	FVector3d CalculateAlignment(ACPP_BoidActor* Boid);

	FVector3d CalculateCohesion(ACPP_BoidActor* Boid);

	void UpdateBoids(TArray<ACPP_BoidActor*> Boids, float DeltaTime);

	void ContainBoids(ACPP_BoidActor* Boid);

};


