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

void ALevelGenerator::GenerateLevel(UStaticMesh* cubeMesh, TSubclassOf<AActor> pelletBP)
{
	// True means there is a wall in that cell
	AStaticMeshActor* level[numRows][numCols]{};

	// Only look at first half of columns in the level array
	// Spawn walls around entire edge. Walls have X% chance to move inward by 1-3 pellets and span 5-10 pellets.
	// Iteration #1: Just walls around the edge
	for (int col = 0; col < numCols / 2; col++) {
		level[0][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, 0, 0), FRotator());
		level[0][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[0][col]->SetActorLabel(TEXT("Wall"));

		level[1][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[1][col]->SetActorLocation(FVector(col * 100, 100, 0));

		level[numRows - 1][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows - 1) * 100, 0), FRotator());
		level[numRows - 1][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[numRows - 1][col]->SetActorLabel(TEXT("Wall"));

		level[numRows - 2][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numRows - 2][col]->SetActorLocation(FVector(col * 100, (numRows - 2) * 100, 0));
	}

	for (int row = 1; row < numRows - 1; row++) {
		level[row][0] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, row * 100, 0), FRotator());
		level[row][0]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[row][0]->SetActorLabel(TEXT("Wall"));

		level[row][1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[row][1]->SetActorLocation(FVector(100, row * 100, 0));

		if (row > numRows / 2 - 3 && row < numRows / 2 + 1) {
			level[row][numCols / 2 - 4] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector((numCols / 2 - 4) * 100, row * 100, 0), FRotator());
			level[row][numCols / 2 - 4]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			level[row][numCols / 2 - 4]->SetActorLabel(TEXT("Wall"));
		}
	}

	// Spawn center area row = 14 col = 13
	for (int col = numCols / 2 - 1; col > numCols / 2 - 5; col--) {
		level[numRows / 2 + 1][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 + 1) * 100, 0), FRotator());
		level[numRows / 2 + 1][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[numRows / 2 + 1][col]->SetActorLabel(TEXT("Wall"));

		if (col != numCols / 2 - 1) {
			level[numRows / 2 - 3][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 - 3) * 100, 0), FRotator());
			level[numRows / 2 - 3][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			level[numRows / 2 - 3][col]->SetActorLabel(TEXT("Wall"));
		}
	}

	// Spawn inner walls
		// Design 1: Spawn X walls that are based on templates. They are allowed to overlap.
		// Design 2: Select X random points on the level. Cubes randomly wander for Y spaces.
		// Design 3: Place X random points in the level. Manhattan pathfind between each to create paths.
		// Design 4: Choose X lanes to cross horizontally and Y lanes to cross vertically. Consider trying to block off some intersections

		// Design 5:
			// Choose X points to start branches from
			// Paths branch out randomly from the edge path and wander Y times.
			// From each path's endpoint, extend the path out vertically and horizontally.
			// All other spaces are walls.



	// Search level for any cell with 3 surrounding walls
		// Dig in the direction of the wall opposite the opening until an edge or opening is found.
		// If a level edge is reached (edge is :
			// If there are less than 2 tunnels:
				// Create a tunnel at that edge. Increment number of tunnels.

	for (int row = 0; row < numRows; row++) {
		for (int col = numCols / 2; col < numCols; col++) {
			level[row][col] = level[row][numCols - col - 1];

			if (level[row][numCols - col - 1] != nullptr) {
				if (level[row][numCols - col - 1]->GetActorLabel() == TEXT("Wall")) {
					level[row][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, row * 100, 0), FRotator());
					level[row][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
				}
				else {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 0));
				}
			}
		}
	}
}