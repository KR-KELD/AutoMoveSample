// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "AutoMoveController.h"

void AMyPlayerController::StartAutoMove(FVector InDestination)
{
	if (!IsValid(AutoMoveController))
	{
		return;
	}

	AutoMoveController->ConnectPawn(GetPawn());
	AutoMoveController->StartAutoMove(InDestination);
}

void AMyPlayerController::StopAutoMove()
{
	if (!IsValid(AutoMoveController))
	{
		return;
	}

	if (AutoMoveController->IsAutoMove())
	{
		AutoMoveController->StopAutoMove();
	}
}

void AMyPlayerController::StartAutoMoveTest(FVector InDestination)
{
	StartAutoMove(InDestination);
}

void AMyPlayerController::StopAutoMoveTest()
{
	StopAutoMove();
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 자동이동 컨트롤러 생성
	if (AutoMoveControllerClass)
	{
		AutoMoveController = GetWorld()->SpawnActor<AAutoMoveController>(AutoMoveControllerClass);
		AutoMoveController->SetActorHiddenInGame(true);
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);
		AutoMoveController->AttachToActor(GetPawn(), AttachmentRules);
		AutoMoveController->InitializeBehaviorTree();
	}
}
