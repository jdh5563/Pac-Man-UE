// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

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

void ALevelGenerator::ToggleLevelActive(int index, bool isActive) {
	for (int i = 0; i < levels[index].size(); i++) {
		if (levels[index][i] != nullptr) {
			levels[index][i]->SetActorHiddenInGame(!isActive);
			levels[index][i]->SetActorEnableCollision(isActive);
			levels[index][i]->SetActorTickEnabled(isActive);
		}
	}
}

void ALevelGenerator::DuplicateLevel(int index) {
	levels.push_back(std::vector<AStaticMeshActor*>());

	for (int i = 0; i < levels[index].size(); i++) {
		//std::copy(levels[levels.size() - 1], levels[index], levels[index].size());
		std::copy(levels[index].begin(), levels[index].end(), levels[levels.size() - 1].begin());
	}
}

TArray<AStaticMeshActor*> ALevelGenerator::GenerateLevel(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP, TSubclassOf<AActor> teleportBP)
{
	// True means there is a wall in that cell
	for (int i = 0; i < numRows * numCols; i++) {
		level.Add(nullptr);
	}

	// Only look at first half of columns in the &level array
	// Spawn walls around entire edge. Walls have X% chance to move inward by 1-3 pellets and span 5-10 pellets.
	// Iteration #1: Just walls around the edge
	BuildLevelOutline(wall, pelletBP, powerPelletBP);

	// Spawn inner walls
		// Design 5:
			// Choose X points to start branches from
			// Paths branch out randomly from the edge path and wander Y times.
			// All other spaces are walls.

	for (int i = 0; i < numBranches; i++) {
		FVector randomPoint;
		int startDir;
		int prevDir; // 0 = LEFT, 1 = UP, 2 = RIGHT, 3 = DOWN

		if (i < numBranches / 4) {
			randomPoint = FVector(numRows - 2, FMath::RandRange(2, numCols / 2 - 3), 0);
			startDir = 3;
			prevDir = 3;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.X -= 1;
				if (level[numCols * (int)randomPoint.X + (int)randomPoint.Y] == nullptr) {
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
				}
				else {
					randomPoint.X += 1;
					break;
				}
			}
		}
		else if (i < numBranches / 2) {
			randomPoint = FVector(FMath::RandRange(2, numRows / 2 - 2), 1, 0);
			startDir = 2;
			prevDir = 2;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.Y += 1;
				if (level[numCols * (int)randomPoint.X + (int)randomPoint.Y] == nullptr) {
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
				}
				else {
					randomPoint.Y -= 1;
					break;
				}
			}
		}
		else if (i < numBranches * 3 / 4) {
			randomPoint = FVector(FMath::RandRange(numRows / 2, numRows - 3), 1, 0);
			startDir = 2;
			prevDir = 2;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.Y += 1;
				if (level[numCols * (int)randomPoint.X + (int)randomPoint.Y] == nullptr) {
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
				}
				else {
					randomPoint.Y -= 1;
					break;
				}
			}
		}
		else {
			randomPoint = FVector(1, FMath::RandRange(2, numCols / 2 - 3), 0);
			startDir = 1;
			prevDir = 1;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.X += 1;
				if (level[numCols * (int)randomPoint.X + (int)randomPoint.Y] == nullptr) {
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[numCols * (int)randomPoint.X + (int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
				}
				else {
					randomPoint.X -= 1;
					break;
				}
			}
		}

		HandlePelletWander(pelletBP, startDir, prevDir, randomPoint);
	}

	// Search level for any cell with 3 surrounding walls
		// Dig in the direction of the wall opposite the opening until an edge or opening is found.
		// If a level edge is reached (edge is :
			// If there are less than 2 tunnels:
				// Create a tunnel at that edge. Increment number of tunnels.

	CullWallsAndPellets(wall, pelletBP);
	FillEmptySpace(wall, pelletBP, powerPelletBP, teleportBP);

	return level;
}

void ALevelGenerator::HandlePelletWander(TSubclassOf<AActor> pelletBP, int startDir, int prevDir, FVector randomPoint)
{
	for (int j = 0; j < numWanders; j++) {
		switch (prevDir) {
		case 0:
			if (startDir != 2 && FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 1;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else {
				prevDir = 3;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X += 1;
						break;
					}
				}
			}
			break;
		case 1:
			if (startDir != 3 && FMath::RandBool()) {
				prevDir = 1;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else {
				prevDir = 2;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y -= 1;
						break;
					}
				}
			}
			break;
		case 2:
			if (FMath::RandBool()) {
				prevDir = 2;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y -= 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 1;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else {
				prevDir = 3;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X += 1;
						break;
					}
				}
			}
			break;
		case 3:
			if (startDir != 1 && FMath::RandBool()) {
				prevDir = 3;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X += 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else {
				prevDir = 2;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y += 1;

					if (!TryWander(pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y -= 1;
						break;
					}
				}
			}
			break;
		}
	}
}

bool ALevelGenerator::TryWander(TSubclassOf<AActor> pelletBP, int row, int col) {
	if (level[numCols * row + col] == nullptr && col < numCols / 2 && !(row > numRows / 2 - 4 && row < numRows / 2 + 2 && numCols - col - 1 > numCols / 2 - 5 && numCols - col - 1 < numCols / 2)) {
		level[numCols * row + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numCols * row + col]->SetActorLocation(FVector(col * 100, row * 100, 200));

		return true;
	}

	return false;
}

void ALevelGenerator::BuildLevelOutline(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP) {
	for (int col = 0; col < numCols / 2; col++) {
		level[col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[col]->SetActorLocation(FVector(col * 100, 0, 200));

		level[numCols * (numRows - 1) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[numCols * (numRows - 1) + col]->SetActorLocation(FVector(col * 100, (numRows - 1) * 100, 200));

		if (col > 1) {
			level[numCols + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numCols + col]->SetActorLocation(FVector(col * 100, 100, 200));
			level[numCols * (numRows - 2) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numCols * (numRows - 2) + col]->SetActorLocation(FVector(col * 100, (numRows - 2) * 100, 200));
		}
	}

	for (int row = 1; row < numRows - 1; row++) {
		level[numCols * row] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[numCols * row]->SetActorLocation(FVector(0, row * 100, 200));

		if (row != 1 && row != numRows - 2) {
			level[numCols * row + 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numCols * row + 1]->SetActorLocation(FVector(100, row * 100, 200));
		}
		else {
			level[numCols * row + 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(powerPelletBP.Get());
			level[numCols * row + 1]->SetActorLocation(FVector(100, row * 100, 200));
		}

		if (row > numRows / 2 - 3 && row < numRows / 2 + 1) {
			level[numCols * row + (numCols / 2 - 4)] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
			level[numCols * row + (numCols / 2 - 4)]->SetActorLocation(FVector((numCols / 2 - 4) * 100, row * 100, 200));

			level[numCols * row + (numCols / 2 - 5)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numCols * row + (numCols / 2 - 5)]->SetActorLocation(FVector((numCols / 2 - 5) * 100, row * 100, 200));
		}
	}

	level[numCols * (numRows / 2 - 3) + (numCols / 2 - 5)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	level[numCols * (numRows / 2 - 3) + (numCols / 2 - 5)]->SetActorLocation(FVector((numCols / 2 - 5) * 100, (numRows / 2 - 3) * 100, 200));
	level[numCols * (numRows / 2 + 1) + (numCols / 2 - 5)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	level[numCols * (numRows / 2 + 1) + (numCols / 2 - 5)]->SetActorLocation(FVector((numCols / 2 - 5) * 100, (numRows / 2 + 1) * 100, 200));

	// Spawn center area row = 14 col = 13
	for (int col = numCols / 2 - 1; col > numCols / 2 - 5; col--) {
		level[numCols * (numRows / 2 + 1) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[numCols * (numRows / 2 + 1) + col]->SetActorLocation(FVector(col * 100, (numRows / 2 + 1) * 100, 200));

		if (col != numCols / 2 - 1) {
			level[numCols * (numRows / 2 - 3) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
			level[numCols * (numRows / 2 - 3) + col]->SetActorLocation(FVector(col * 100, (numRows / 2 - 3) * 100, 200));
		}
	}

	for (int col = numCols / 2 - 1; col > numCols / 2 - 6; col--) {
		level[numCols * (numRows / 2 + 2) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numCols * (numRows / 2 + 2) + col]->SetActorLocation(FVector(col * 100, (numRows / 2 + 2) * 100, 200));

		level[numCols * (numRows / 2 - 4) + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numCols * (numRows / 2 - 4) + col]->SetActorLocation(FVector(col * 100, (numRows / 2 - 4) * 100, 200));
	}
}

void ALevelGenerator::CullWallsAndPellets(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP) {

	for (int row = 0; row < numRows; row++) {
		for (int col = numCols / 2; col < numCols; col++) {
			if (row > 1 && row < numRows - 2 && col > 1 && col < numCols - 2) {
				if (IsPelletSurrounded(row, numCols - col - 1)) {
					level[numCols * row + (numCols - col - 1)]->Destroy();
					level[numCols * row + (numCols - col - 1)] = nullptr;
				}
				else if (!(row > numRows / 2 - 4 && row < numRows / 2 + 2 && numCols - col - 1 > numCols / 2 - 5 && numCols - col - 1 < numCols / 2)) {
					switch (IsDeadEnd(row, numCols - col - 1)) {
					case 1:
						for (int i = numCols - col - 2; i > 1; i--) {
							if (level[numCols * row + i] != nullptr) level[numCols * row + i]->Destroy();
							level[numCols * row + i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * row + i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[numCols * row + (i - 1)] != nullptr && !level[numCols * row + (i - 1)]->Tags.Contains(TEXT("Wall"))) break;
						}
						break;
					case 2:
						for (int i = row - 1; i > 1; i--) {
							if (level[numCols * i + (numCols - col - 1)] != nullptr) level[numCols * i + (numCols - col - 1)]->Destroy();
							level[numCols * i + (numCols - col - 1)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * i + (numCols - col - 1)]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[numCols * (i - 1) + (numCols - col - 1)] != nullptr && !level[numCols * (i - 1) + (numCols - col - 1)]->Tags.Contains(TEXT("Wall"))) break;
						}
						break;
					case 3:
						for (int i = numCols - col; i < numCols / 2; i++) {
							if (level[numCols * row + i] != nullptr) level[numCols * row + i]->Destroy();
							level[numCols * row + i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * row + i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[numCols * row + (i + 1)] != nullptr && !level[numCols * row + (i + 1)]->Tags.Contains(TEXT("Wall"))) break;
						}
						break;
					case 4:
						for (int i = row + 1; i < numRows - 2; i++) {
							if (level[numCols * i + (numCols - col - 1)] != nullptr) level[numCols * i + (numCols - col - 1)]->Destroy();
							level[numCols * i + (numCols - col - 1)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * i + (numCols - col - 1)]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[numCols * (i + 1) + (numCols - col - 1)] != nullptr && !level[numCols * (i + 1) + (numCols - col - 1)]->Tags.Contains(TEXT("Wall"))) break;
						}
						break;
					case 5:
						for (int i = numCols - col - 2; i > 1; i--) {
							if (level[numCols * row + i] != nullptr) level[numCols * row + i]->Destroy();
							level[numCols * row + i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * row + i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[numCols * row + (i - 1)] != nullptr && !level[numCols * row + (i - 1)]->Tags.Contains(TEXT("Wall"))) break;
						}

						for (int i = row - 1; i > 1; i--) {
							if (level[numCols * i + (numCols - col - 1)] != nullptr) level[numCols * i + (numCols - col - 1)]->Destroy();
							level[numCols * i + (numCols - col - 1)] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[numCols * i + (numCols - col - 1)]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[numCols * (i - 1) + (numCols - col - 1)] != nullptr && !level[numCols * (i - 1) + (numCols - col - 1)]->Tags.Contains(TEXT("Wall"))) break;
						}
						break;
					}
				}
			}
		}
	}
}

void ALevelGenerator::FillEmptySpace(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP, TSubclassOf<AActor> teleportBP) {
	levels.push_back(std::vector<AStaticMeshActor*>());

	int tunnel1Index = FMath::RandRange(1, numRows - 2);
	int tunnel2Index = FMath::RandRange(1, numRows - 2);
	while (FMath::Abs(tunnel2Index - tunnel1Index) < 2) {
		tunnel2Index = FMath::RandRange(1, numRows - 2);
	}

	for (int row = 0; row < numRows; row++) {
		if (row == tunnel1Index || row == tunnel2Index) {
			level[numCols * row]->Destroy();
			level[numCols * row] = nullptr;

			for (int i = 1; i < 3; i++) {
				// Left-side teleporter
				AStaticMeshActor* hallway = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
				hallway->SetActorLocation(FVector(-i * 100, (row + 1) * 100, 200));
				levels[levels.size() - 1].push_back(hallway);

				hallway = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
				hallway->SetActorLocation(FVector(-i * 100, (row - 1) * 100, 200));
				levels[levels.size() - 1].push_back(hallway);

				// Right-side teleporter
				hallway = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
				hallway->SetActorLocation(FVector((numCols - 1 + i) * 100, (row + 1) * 100, 200));
				levels[levels.size() - 1].push_back(hallway);

				hallway = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
				hallway->SetActorLocation(FVector((numCols - 1 + i) * 100, (row - 1) * 100, 200));
				levels[levels.size() - 1].push_back(hallway);
			}

			AStaticMeshActor* teleporter = (AStaticMeshActor*)GetWorld()->SpawnActor(teleportBP.Get());
			teleporter->SetActorLocation(FVector(-300, row * 100, 200));
			teleporter->Tags.Add(TEXT("Left"));
			levels[levels.size() - 1].push_back(teleporter);

			teleporter = (AStaticMeshActor*)GetWorld()->SpawnActor(teleportBP.Get());
			teleporter->SetActorLocation(FVector((numCols + 2) * 100, row * 100, 200));
			levels[levels.size() - 1].push_back(teleporter);
		}

		for (int col = numCols / 2; col < numCols; col++) {
			level[numCols * row + col] = level[numCols * row + (numCols - col - 1)];

			if (level[numCols * row + (numCols - col - 1)] != nullptr) {
				if (level[numCols * row + (numCols - col - 1)]->Tags.Contains(TEXT("Wall"))) {
					level[numCols * row + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[numCols * row + col]->SetActorLocation(FVector(col * 100, row * 100, 200));
				}
				else if (level[numCols * row + (numCols - col - 1)]->Tags.Contains(TEXT("PowerPellet"))) {
					level[numCols * row + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(powerPelletBP.Get());
					level[numCols * row + col]->SetActorLocation(FVector(col * 100, row * 100, 200));
				}
				else {
					level[numCols * row + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[numCols * row + col]->SetActorLocation(FVector(col * 100, row * 100, 200));
				}
			}
			else if (!(numCols - col - 1 < numCols / 2 && numCols - col - 1 > numCols / 2 - 5 && row < numRows / 2 + 2 && row > numRows / 2 - 4)) {
				if (col != numCols - 1 || (row != tunnel1Index && row != tunnel2Index)) {
					level[numCols * row + col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[numCols * row + col]->SetActorLocation(FVector(col * 100, row * 100, 200));

					level[numCols * row + (numCols - col - 1)] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[numCols * row + (numCols - col - 1)]->SetActorLocation(FVector((numCols - col - 1) * 100, row * 100, 200));
				}
			}

			levels[levels.size() - 1].push_back(level[numCols * row + (numCols - col - 1)]);
			levels[levels.size() - 1].push_back(level[numCols * row + col]);
		}
	}
}

bool ALevelGenerator::IsPelletSurrounded(int row, int col) {
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			if (level[numCols * i + j] == nullptr || level[numCols * i + j]->Tags.Contains(TEXT("Wall"))) return false;
		}
	}

	return true;
}

int ALevelGenerator::IsDeadEnd(int row, int col) {
	int numWalls = 0;
	int digDir = 1;
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			if ((i == row || j == col) && (i != row || j != col)) {
				if (level[numCols * i + j] == nullptr || level[numCols * i + j]->Tags.Contains(TEXT("Wall"))) numWalls++;
				else {
					if (i == row) {
						if (j == col - 1) digDir = 3;
						else digDir = 1;
					}
					else {
						if (i == row - 1) digDir = 4;
						else digDir = 2;
					}
				}
			}
		}
	}

	if (numWalls == 4) digDir = 5;
	return numWalls < 3 ? 0 : digDir;
}