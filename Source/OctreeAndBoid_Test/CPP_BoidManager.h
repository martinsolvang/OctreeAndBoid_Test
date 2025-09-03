// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_BoidManager.generated.h"

class ACPP_BoidActor;

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_BoidManager : public AActor
{
	GENERATED_BODY()

	TArray<ACPP_BoidActor*> BoidActors;
	
	UPROPERTY(EditAnywhere)
	float SeparationWeight;

	UPROPERTY(EditAnywhere)
	float ALignmentWeight;

	UPROPERTY(EditAnywhere)
	float CohesionWeight;

	UPROPERTY(EditAnywhere)
	float VisionRadius;
	
public:
	// Sets default values for this actor's properties
	ACPP_BoidManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PopulateBoidList();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CalculateSeparation();

	void CalculateAlignment();
	
	void CalculateCohesion();

	void UpdateBoids();
};
