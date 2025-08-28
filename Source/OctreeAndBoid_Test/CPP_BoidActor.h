// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_BoidActor.generated.h"

UCLASS()
class OCTREEANDBOID_TEST_API ACPP_BoidActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_BoidActor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector Velocity;

	void UpdateBoid(const FVector& NewVelocity, float DeltaTime);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	

};
