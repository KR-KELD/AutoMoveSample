// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AutoMoveController.generated.h"

class AMyPlayerController;

/**
 * 
 */
UCLASS()
class PLAYERAISAMPLE_API AAutoMoveController : public AAIController
{
	GENERATED_BODY()
public:
	AAutoMoveController(const FObjectInitializer& ObjectInitializer);
public:

	void InitializeBehaviorTree();
	void ConnectPawn(APawn* InPawn);
	
	UBehaviorTreeComponent* GetBehaviorTreeComponent() const;

	void StartAutoMove(FVector InDestination);
	void StopAutoMove();

	bool IsAutoMove();

	bool IsPossibleAutoMove(FVector InDestination);

	AMyPlayerController* GetPlayerController() { return ConnectedPlayerController; }
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Move")
	UBehaviorTree* AutoMoveBehaviorTree;

private:
	UPROPERTY()
	AMyPlayerController* ConnectedPlayerController;

	bool IsAutoMoving;
};
