// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "FSpatialHashGrid.generated.h"

USTRUCT()
struct FSpatialHashGrid
{

	GENERATED_BODY()
	
	FSpatialHashGrid(float CellSize, bool bUseNeighborCells);
	FSpatialHashGrid() = default;

	private:

	

	TMap<FIntVector, TArray<int32>> Cells;

	FIntVector GetCellVector(const FVector& Position) const;


public:
	UPROPERTY(EditAnywhere, Category="Grid Settings")
	float CellSize;
	
	bool bUseNeighborCells = true;

	//Clears the grid of all boids 
	void ClearGrid();

	
	void InsertBoid(int32 BoidIndex, const FVector& Position);
	void GetNeighbourBoids(const FVector& Position, TArray<int32>& OutIndices) const;

	void DrawGrid(UWorld* World, const FColor& Color = FColor::Green, float Duration = 0.0f) const;

};
