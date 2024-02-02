// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

UCLASS()
class PACMAN_API ALevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditInstanceOnly)
	int numBranches = 8;

	UPROPERTY(EditInstanceOnly)
	int wanderDistance = 7;

	UPROPERTY(EditInstanceOnly)
	int numWanders = 3;

	const static int numRows = 31;
	const static int numCols = 28;

	UFUNCTION(BlueprintCallable)
	void GenerateLevel(TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP);

	UFUNCTION(BlueprintCallable)
	void DuplicateLevel(int index);

	UFUNCTION(BlueprintCallable)
	void ToggleLevelActive(int index, bool isActive);

	bool IsPelletSurrounded(AStaticMeshActor* level[numRows][numCols], int row, int col);

	// 0 = NOT DEAD END, 1 = LEFT, 2 = UP, 3 = RIGHT, 4 = DOWN, 5 = LEFT + UP
	int IsDeadEnd(AStaticMeshActor* level[numRows][numCols], int row, int col);

	void FillEmptySpace(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP);

	void CullWallsAndPellets(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP);

	void BuildLevelOutline(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> wall, TSubclassOf<AActor> pelletBP, TSubclassOf<AActor> powerPelletBP);

	bool TryWander(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> pelletBP, int row, int col);

	void HandlePelletWander(AStaticMeshActor* level[numRows][numCols], TSubclassOf<AActor> pelletBP, int startDir, int prevDir, FVector randomPoint);

	std::vector<std::vector<AStaticMeshActor*>> levels;
};
