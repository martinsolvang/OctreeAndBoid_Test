// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_BoidActor.h"

// Sets default values
ACPP_BoidActor::ACPP_BoidActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACPP_BoidActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACPP_BoidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

