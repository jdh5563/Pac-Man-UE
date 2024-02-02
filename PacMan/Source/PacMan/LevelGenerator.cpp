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

void ALevelGenerator::ToggleLevelHidden(int index, bool isHidden) {
	for (int i = 0; i < levels[index].size(); i++) {
		levels[index][i]->SetActorHiddenInGame(isHidden);
	}
}

void ALevelGenerator::GenerateLevel(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP)
{
	// True means there is a wall in that cell
	AStaticMeshActor* level[numRows][numCols]{};

	// Only look at first half of columns in the level array
	// Spawn walls around entire edge. Walls have X% chance to move inward by 1-3 pellets and span 5-10 pellets.
	// Iteration #1: Just walls around the edge
	BuildLevelOutline(level, wall, pelletBP, powerPelletBP);

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
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}
		else if (i < numBranches / 2) {
			randomPoint = FVector(FMath::RandRange(2, numRows / 2 - 2), 1, 0);
			startDir = 2;
			prevDir = 2;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.Y += 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}
		else if (i < numBranches * 3 / 4) {
			randomPoint = FVector(FMath::RandRange(numRows / 2, numRows - 3), 1, 0);
			startDir = 2;
			prevDir = 2;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.Y += 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}
		else {
			randomPoint = FVector(1, FMath::RandRange(2, numCols / 2 - 3), 0);
			startDir = 1;
			prevDir = 1;
			for (int j = 0; j < wanderDistance; j++) {
				randomPoint.X += 1;
				level[(int)randomPoint.X][(int)randomPoint.Y] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
				level[(int)randomPoint.X][(int)randomPoint.Y]->SetActorLocation(FVector(randomPoint.Y * 100, randomPoint.X * 100, 200));
			}
		}

		HandlePelletWander(level, pelletBP, startDir, prevDir, randomPoint);
	}

	// Search level for any cell with 3 surrounding walls
		// Dig in the direction of the wall opposite the opening until an edge or opening is found.
		// If a level edge is reached (edge is :
			// If there are less than 2 tunnels:
				// Create a tunnel at that edge. Increment number of tunnels.

	CullWallsAndPellets(level, wall, pelletBP);
	FillEmptySpace(level, wall, pelletBP, powerPelletBP);
}

void ALevelGenerator::HandlePelletWander(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> pelletBP, int startDir, int prevDir, FVector randomPoint)
{
	for (int j = 0; j < numWanders; j++) {
		switch (prevDir) {
		case 0:
			if (startDir != 2 && FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 1;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X += 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else {
				prevDir = 3;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X -= 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
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

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else {
				prevDir = 2;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y += 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
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

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y -= 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 1;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X += 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X -= 1;
						break;
					}
				}
			}
			else {
				prevDir = 3;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.X -= 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
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

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.X += 1;
						break;
					}
				}
			}
			else if (FMath::RandBool()) {
				prevDir = 0;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y -= 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y += 1;
						break;
					}
				}
			}
			else {
				prevDir = 2;
				for (int k = 0; k < wanderDistance; k++) {
					randomPoint.Y += 1;

					if (!TryWander(level, pelletBP, randomPoint.X, randomPoint.Y)) {
						randomPoint.Y -= 1;
						break;
					}
				}
			}
			break;
		}
	}
}

bool ALevelGenerator::TryWander(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> pelletBP, int row, int col) {
	if (level[row][col] == nullptr && col < numCols / 2 && !(row > numRows / 2 - 4 && row < numRows / 2 + 2 && numCols - col - 1 > numCols / 2 - 5 && numCols - col - 1 < numCols / 2)) {
		level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));

		return true;
	}

	return false;
}

void ALevelGenerator::BuildLevelOutline(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP) {
	for (int col = 0; col < numCols / 2; col++) {
		level[0][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[0][col]->SetActorLocation(FVector(col * 100, 0, 200));
		level[0][col]->SetActorLabel(TEXT("Wall"));

		level[numRows - 1][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[numRows - 1][col]->SetActorLocation(FVector(col * 100, (numRows - 1) * 100, 200));
		level[numRows - 1][col]->SetActorLabel(TEXT("Wall"));

		if (col != 0) {
			level[1][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[1][col]->SetActorLocation(FVector(col * 100, 100, 200));
			level[numRows - 2][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[numRows - 2][col]->SetActorLocation(FVector(col * 100, (numRows - 2) * 100, 200));
		}
	}

	for (int row = 1; row < numRows - 1; row++) {
		level[row][0] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[row][0]->SetActorLocation(FVector(0, row * 100, 200));
		level[row][0]->SetActorLabel(TEXT("Wall"));

		if (row != 1 && row != numRows - 2) {
			level[row][1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[row][1]->SetActorLocation(FVector(100, row * 100, 200));
		}
		else {
			level[row][1] = (AStaticMeshActor*)GetWorld()->SpawnActor(powerPelletBP.Get());
			level[row][1]->SetActorLocation(FVector(100, row * 100, 200));
			level[row][1]->SetActorLabel(TEXT("PowerPellet"));
		}

		if (row > numRows / 2 - 3 && row < numRows / 2 + 1) {
			level[row][numCols / 2 - 4] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
			level[row][numCols / 2 - 4]->SetActorLocation(FVector((numCols / 2 - 4) * 100, row * 100, 200));
			level[row][numCols / 2 - 4]->SetActorLabel(TEXT("Wall"));

			level[row][numCols / 2 - 5] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
			level[row][numCols / 2 - 5]->SetActorLocation(FVector((numCols / 2 - 5) * 100, row * 100, 200));
		}
	}

	level[numRows / 2 - 3][numCols / 2 - 5] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	level[numRows / 2 - 3][numCols / 2 - 5]->SetActorLocation(FVector((numCols / 2 - 5) * 100, (numRows / 2 - 3) * 100, 200));
	level[numRows / 2 + 1][numCols / 2 - 5] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
	level[numRows / 2 + 1][numCols / 2 - 5]->SetActorLocation(FVector((numCols / 2 - 5) * 100, (numRows / 2 + 1) * 100, 200));

	// Spawn center area row = 14 col = 13
	for (int col = numCols / 2 - 1; col > numCols / 2 - 5; col--) {
		level[numRows / 2 + 1][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
		level[numRows / 2 + 1][col]->SetActorLocation(FVector(col * 100, (numRows / 2 + 1) * 100, 200));
		level[numRows / 2 + 1][col]->SetActorLabel(TEXT("Wall"));

		if (col != numCols / 2 - 1) {
			level[numRows / 2 - 3][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
			level[numRows / 2 - 3][col]->SetActorLocation(FVector(col * 100, (numRows / 2 - 3) * 100, 200));
			level[numRows / 2 - 3][col]->SetActorLabel(TEXT("Wall"));
		}
	}

	for (int col = numCols / 2 - 1; col > numCols / 2 - 6; col--) {
		level[numRows / 2 + 2][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numRows / 2 + 2][col]->SetActorLocation(FVector(col * 100, (numRows / 2 + 2) * 100, 200));

		level[numRows / 2 - 4][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
		level[numRows / 2 - 4][col]->SetActorLocation(FVector(col * 100, (numRows / 2 - 4) * 100, 200));
	}
}

void ALevelGenerator::CullWallsAndPellets(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP) {

	for (int row = 0; row < numRows; row++) {
		for (int col = numCols / 2; col < numCols; col++) {
			if (row > 1 && row < numRows - 2 && col > 1 && col < numCols - 2) {
				if (IsPelletSurrounded(level, row, numCols - col - 1)) {
					level[row][numCols - col - 1]->Destroy();
					level[row][numCols - col - 1] = nullptr;
				}
				else if (!(row > numRows / 2 - 4 && row < numRows / 2 + 2 && numCols - col - 1 > numCols / 2 - 5 && numCols - col - 1 < numCols / 2)) {
					switch (IsDeadEnd(level, row, numCols - col - 1)) {
					case 1:
						for (int i = numCols - col - 2; i > 1; i--) {
							if (level[row][i] != nullptr) level[row][i]->Destroy();
							level[row][i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[row][i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[row][i - 1] != nullptr && level[row][i - 1]->GetActorLabel() != TEXT("Wall")) break;
						}
						break;
					case 2:
						for (int i = row - 1; i > 1; i--) {
							if (level[i][numCols - col - 1] != nullptr) level[i][numCols - col - 1]->Destroy();
							level[i][numCols - col - 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[i][numCols - col - 1]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[i - 1][numCols - col - 1] != nullptr && level[i - 1][numCols - col - 1]->GetActorLabel() != TEXT("Wall")) break;
						}
						break;
					case 3:
						for (int i = numCols - col; i < numCols / 2; i++) {
							if (level[row][i] != nullptr) level[row][i]->Destroy();
							level[row][i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[row][i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[row][i + 1] != nullptr && level[row][i + 1]->GetActorLabel() != TEXT("Wall")) break;
						}
						break;
					case 4:
						for (int i = row + 1; i < numRows - 2; i++) {
							if (level[i][numCols - col - 1] != nullptr) level[i][numCols - col - 1]->Destroy();
							level[i][numCols - col - 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[i][numCols - col - 1]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[i + 1][numCols - col - 1] != nullptr && level[i + 1][numCols - col - 1]->GetActorLabel() != TEXT("Wall")) break;
						}
						break;
					case 5:
						for (int i = numCols - col - 2; i > 1; i--) {
							if (level[row][i] != nullptr) level[row][i]->Destroy();
							level[row][i] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[row][i]->SetActorLocation(FVector(i * 100, row * 100, 200));

							if (level[row][i - 1] != nullptr && level[row][i - 1]->GetActorLabel() != TEXT("Wall")) break;
						}

						for (int i = row - 1; i > 1; i--) {
							if (level[i][numCols - col - 1] != nullptr) level[i][numCols - col - 1]->Destroy();
							level[i][numCols - col - 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
							level[i][numCols - col - 1]->SetActorLocation(FVector((numCols - col - 1) * 100, i * 100, 200));

							if (level[i - 1][numCols - col - 1] != nullptr && level[i - 1][numCols - col - 1]->GetActorLabel() != TEXT("Wall")) break;
						}
						break;
					}
				}
			}
		}
	}
}

void ALevelGenerator::FillEmptySpace(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP) {
	for (int row = 0; row < numRows; row++) {
		for (int col = numCols / 2; col < numCols; col++) {
			level[row][col] = level[row][numCols - col - 1];

			if (level[row][numCols - col - 1] != nullptr) {
				if (level[row][numCols - col - 1]->GetActorLabel() == TEXT("Wall")) {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));
					level[row][col]->SetActorLabel(TEXT("Wall"));
				}
				else if (level[row][numCols - col - 1]->GetActorLabel() == TEXT("PowerPellet")) {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(powerPelletBP.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));
					level[row][col]->SetActorLabel(TEXT("PowerPellet"));
				}
				else {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(pelletBP.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));
				}
			}
			else {
				if (!(numCols - col - 1 < numCols / 2 && numCols - col - 1 > numCols / 2 - 5 && row < numRows / 2 + 2 && row > numRows / 2 - 4)) {
					level[row][col] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[row][col]->SetActorLocation(FVector(col * 100, row * 100, 200));
					level[row][col]->SetActorLabel(TEXT("Wall"));

					level[row][numCols - col - 1] = (AStaticMeshActor*)GetWorld()->SpawnActor(wall.Get());
					level[row][numCols - col - 1]->SetActorLocation(FVector((numCols - col - 1) * 100, row * 100, 200));
					level[row][numCols - col - 1]->SetActorLabel(TEXT("Wall"));
				}
			}

			levels.push_back(std::vector<AStaticMeshActor*>());
			levels[levels.size() - 1].push_back(level[row][numCols - col - 1]);
			levels[levels.size() - 1].push_back(level[row][col]);
		}
	}
}

bool ALevelGenerator::IsPelletSurrounded(AStaticMeshActor* level[numRows][numCols], int row, int col) {
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			if (level[i][j] == nullptr || level[i][j]->GetActorLabel() == TEXT("Wall")) return false;
		}
	}

	return true;
}

int ALevelGenerator::IsDeadEnd(AStaticMeshActor* level[numRows][numCols], int row, int col) {
	int numWalls = 0;
	int digDir = 1;
	for (int i = row - 1; i < row + 2; i++) {
		for (int j = col - 1; j < col + 2; j++) {
			if ((i == row || j == col) && (i != row || j != col)) {
				if (level[i][j] == nullptr || level[i][j]->GetActorLabel() == TEXT("Wall")) numWalls++;
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