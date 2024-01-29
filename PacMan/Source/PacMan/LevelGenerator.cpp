// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"
#include "Engine/StaticMeshActor.h"

// Sets default values
ALevelGenerator::ALevelGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelGenerator::GenerateLevel(UStaticMesh* cubeMesh)
{
	// True means there is a wall in that cell
	bool level[numRows][numCols]{};

	// Only look at first half of columns in the level array
	// Spawn walls around entire edge. Walls have X% chance to move inward by 1-3 pellets and span 5-10 pellets.
	// Iteration #1: Just walls around the edge
	for (int col = 0; col < numCols / 2; col++) {
		level[0][col] = true;
		level[numRows - 1][col] = true;

		AStaticMeshActor* wall1 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, 0, 0), FRotator());
		wall1->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);

		AStaticMeshActor* wall2 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows - 1) * 100, 0), FRotator());
		wall2->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
	}

	for (int row = 1; row < numRows - 1; row++) {
		level[row][0] = true;

		AStaticMeshActor* wall1 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, row * 100, 0), FRotator());
		wall1->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);

		if (row > numRows / 2 - 3 && row < numRows / 2 + 1) {
			level[row][numCols / 2 - 4] = true;

			AStaticMeshActor* wall2 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector((numCols / 2 - 4) * 100, row * 100, 0), FRotator());
			wall2->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		}
	}

	// Spawn center area row = 14 col = 13
	for (int col = numCols / 2 - 1; col > numCols / 2 - 5; col--) {
		level[numRows / 2 + 1][col] = true;

		AStaticMeshActor* wall1 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 + 1) * 100, 0), FRotator());
		wall1->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);

		if (col != numCols / 2 - 1) {
			level[numRows / 2 - 3][col] = true;

			AStaticMeshActor* wall2 = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 - 3) * 100, 0), FRotator());
			wall2->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		}
	}

	// Spawn inner walls
		// Design 1: Spawn X walls that are based on templates. They are allowed to overlap.
		// Design 2: Select X random points on the level. Cubes randomly wander for Y spaces.
		// Design 3: Place X random points in the level. Manhattan pathfind between each to create paths.
		// Design 4: Choose X lanes to cross horizontally and Y lanes to cross vertically. Consider trying to block off some intersections

	// Search level for any cell with 3 surrounding walls
		// Dig in the direction of the wall opposite the opening until an edge or opening is found.
		// If a level edge is reached (edge is :
			// If there are less than 2 tunnels:
				// Create a tunnel at that edge. Increment number of tunnels.

	for (int row = 0; row < numRows; row++) {
		for (int col = numCols / 2; col < numCols; col++) {
			level[row][col] = level[row][numCols - col - 1];

			if (level[row][col]) {
				AStaticMeshActor* wall = (AStaticMeshActor*)GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, row * 100, 0), FRotator());
				wall->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			}
		}
	}
}