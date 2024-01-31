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
		level[0][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, 0, 200), FRotator());
		level[0][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[0][col]->SetActorLabel(TEXT("Wall"));

		level[numRows - 1][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows - 1) * 100, 200), FRotator());
		level[numRows - 1][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[numRows - 1][col]->SetActorLabel(TEXT("Wall"));

		if (col != 0) {
			level[1][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[1][col]->SetActorLocation(FVector(col * 100, 100, 200));
			level[numRows - 2][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numRows - 2][col]->SetActorLocation(FVector(col * 100, (numRows - 2) * 100, 200));
		}
	}

	for (int row = 1; row < numRows - 1; row++) {
		level[row][0] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0, row * 100, 200), FRotator());
		level[row][0]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[row][0]->SetActorLabel(TEXT("Wall"));

		level[row][1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[row][1]->SetActorLocation(FVector(100, row * 100, 200));

		if (row > numRows / 2 - 3 && row < numRows / 2 + 1) {
			level[row][numCols / 2 - 4] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector((numCols / 2 - 4) * 100, row * 100, 200), FRotator());
			level[row][numCols / 2 - 4]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			level[row][numCols / 2 - 4]->SetActorLabel(TEXT("Wall"));
		}
	}

	// Spawn center area row = 14 col = 13
	for (int col = numCols / 2 - 1; col > numCols / 2 - 5; col--) {
		level[numRows / 2 + 1][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 + 1) * 100, 200), FRotator());
		level[numRows / 2 + 1][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
		level[numRows / 2 + 1][col]->SetActorLabel(TEXT("Wall"));

		if (col != numCols / 2 - 1) {
			level[numRows / 2 - 3][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, (numRows / 2 - 3) * 100, 200), FRotator());
			level[numRows / 2 - 3][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			level[numRows / 2 - 3][col]->SetActorLabel(TEXT("Wall"));
		}
	}

	// Spawn center pellet line = 14 col = 13
	for (int col = numCols / 2 - 1; col > 1; col--) {
		level[numRows / 2 - 4][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numRows / 2 - 4][col]->SetActorLocation(FVector(col * 100, (numRows / 2 - 4) * 100, 200));
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
	const int numBranches = 7;
	int wanderDistance = 1;
	int numWanders = 20;

	FVector endPoints[numBranches];

	for (int i = 0; i < numBranches; i++) {
		FVector randomPoint;
		int startDir;
		int prevDir; // 0 = LEFT, 1 = UP, 2 = RIGHT, 3 = DOWN
		if (FMath::RandBool()) {
			randomPoint = FVector(FMath::RandRange(2, numRows - 3), 1, 0);
			startDir = 2;
			prevDir = 2;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.Y += 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}
		else if (FMath::RandBool()) {
			randomPoint = FVector(1, FMath::RandRange(2, numCols / 2 - 3), 0);
			startDir = 1;
			prevDir = 1;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.X += 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}
		else {
			randomPoint = FVector(numRows - 2, FMath::RandRange(2, numCols / 2 - 3), 0);
			startDir = 3;
			prevDir = 3;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.X -= 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}

		for (int j = 0; j < numWanders; j++) {
			switch (prevDir) {
				case 0:
					if (startDir != 2 && FMath::RandBool()) {
						prevDir = 0;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y -= 1;

							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else if (FMath::RandBool()) {
						prevDir = 1;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else {
						prevDir = 3;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X -= 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					break;
				case 1:
					if (startDir != 3 && FMath::RandBool()) {
						prevDir = 1;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else if (FMath::RandBool()) {
						prevDir = 0;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y -= 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else {
						prevDir = 2;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					break;
				case 2:
					if (FMath::RandBool()) {
						prevDir = 2;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else if (FMath::RandBool()) {
						prevDir = 1;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else {
						prevDir = 3;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X -= 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					break;
				case 3:
					if (startDir != 1 && FMath::RandBool()) {
						prevDir = 3;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.X -= 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.X += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else if (FMath::RandBool()) {
						prevDir = 0;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y -= 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y += 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					else {
						prevDir = 2;
						for (int k = 0; k < wanderDistance; k++) {
							randomPoint.Y += 1;
							if (level[(int)randomPoint.X][(int)randomPoint.Y] == nullptr) {
								level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
								level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
							}
							else {
								randomPoint.Y -= 1;
								endPoints[i] = randomPoint;
							}
						}
					}
					break;
			}
		}
	}

	//for (int i = 0; i < numBranches; i++) {
	//	for (int row = 2; row < numRows - 2; row++) {
	//		if (endPoints[i].Y < numCols / 2 - 4 || !(row > numRows / 2 - 3 && row < numRows / 2 + 1)) {
	//			if (level[row][(int)endPoints[i].Y] == nullptr) {
	//				level[row][(int)endPoints[i].Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	//				level[row][(int)endPoints[i].Y]->SetActorLocation(FVector(endPoints[i].Y * 100, row * 100, 200));
	//			}
	//		}
	//	}

	//	for (int col = 2; col < numCols / 2; col++) {
	//		if (!(endPoints[i].X > numRows / 2 - 3 && endPoints[i].X < numRows / 2 + 1) || col < numCols / 2 - 4) {
	//			if (level[(int)endPoints[i].X][col] == nullptr) {
	//				level[(int)endPoints[i].X][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	//				level[(int)endPoints[i].X][col]->SetActorLocation(FVector(col * 100, endPoints[i].X * 100, 200));
	//			}
	//		}
	//	}
	//}

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
					level[row][col] = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(col * 100, row * 100, 200), FRotator());
					level[row][col]->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
				}
				else {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));
				}
			}
		}
	}
}