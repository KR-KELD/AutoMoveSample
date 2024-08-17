// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class AAutoMoveController;

/**
 * 
 */
UCLASS()
class PLAYERAISAMPLE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void StartAutoMove(FVector InDestination);
	void StopAutoMove();

	UFUNCTION(BlueprintCallable, Category = "Auto Move")
	void StartAutoMoveTest(FVector InDestination);
	UFUNCTION(BlueprintCallable, Category = "Auto Move")
	void StopAutoMoveTest();
protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Move")
	TSubclassOf<AAutoMoveController> AutoMoveControllerClass;

private:
	UPROPERTY()
	AAutoMoveController* AutoMoveController;
};
