// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPinComboBox.h"
#include "GameFramework/Actor.h"
#include "CPP_BoidManager.generated.h"

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_BoidManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACPP_BoidManager();
	
private:
	UPROPERTY(EditAnywhere)
	float SeparationFactor;

	UPROPERTY(EditAnywhere)
	float AlignmentFactor;

	UPROPERTY(EditAnywhere)
	float CohesionFactor;

	//temporary list for calculation
	UPROPERTY(VisibleAnywhere)
	TArray<ACPP_BoidActor*> NearbyBoids;

	UPROPERTY(EditAnywhere)
	float BoidVisionRange;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FindNearestBoids();

	void CalculateSeparation(TArray<ACPP_BoidActor*> Boids);

	void CalculateAlignment(TArray<ACPP_BoidActor*> Boids);

	void CalculateCohesion(TArray<ACPP_BoidActor*> Boids);

	void UpdateBoids(TArray<ACPP_BoidActor*> Boids);
};
