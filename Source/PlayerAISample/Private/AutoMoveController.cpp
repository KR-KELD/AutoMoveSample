// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoMoveController.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "MyPlayerController.h"

AAutoMoveController::AAutoMoveController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	ConnectedPlayerController = nullptr;

	IsAutoMoving = false;
}

void AAutoMoveController::InitializeBehaviorTree()
{
	UBehaviorTreeComponent* BTComp = GetBehaviorTreeComponent();
	if (IsValid(BTComp) && IsValid(AutoMoveBehaviorTree))
	{
		UBlackboardComponent* BBComp = Blackboard;
		UseBlackboard(AutoMoveBehaviorTree->BlackboardAsset, BBComp);
		BBComp->InitializeBlackboard(*AutoMoveBehaviorTree->BlackboardAsset);
	}
}

void AAutoMoveController::ConnectPawn(APawn* InPawn)
{
	if (!IsValid(InPawn))
	{
		return;
	}
	
	// 기존 Possess의 변경 방식이 아닌 추가 연결 방식
	// Pawn 정보만 AIController와 연결시켜 BT에서 사용하기 위함
	SetPawn(InPawn);
	SetControlRotation(InPawn->GetActorRotation());

	AMyPlayerController* MyController = Cast<AMyPlayerController>(InPawn->GetController());
	if (IsValid(MyController))
	{
		ConnectedPlayerController = MyController;
	}

	UPathFollowingComponent* PathFollowingComp = GetPathFollowingComponent();
	if (IsValid(PathFollowingComp))
	{
		PathFollowingComp->Initialize();
	}
}

UBehaviorTreeComponent* AAutoMoveController::GetBehaviorTreeComponent() const
{
	return Cast<UBehaviorTreeComponent>(GetBrainComponent());
}

void AAutoMoveController::StartAutoMove(FVector InDestination)
{
	if (!IsValid(GetPawn()))
	{
		return;
	}

	if (!IsValid(ConnectedPlayerController))
	{
		return;
	}

	if (!IsPossibleAutoMove(InDestination))
	{
		UE_LOG(LogTemp, Log, TEXT("Path not found."));
		return;
	}

	UBehaviorTreeComponent* BTComp = GetBehaviorTreeComponent();
	if (!IsValid(BTComp))
	{
		return;
	}

	if (!IsValid(AutoMoveBehaviorTree))
	{
		return;
	}

	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (!IsValid(BBComp))
	{
		return;
	}

	if (BBComp->IsValidKey(BBComp->GetKeyID(TEXT("Destination"))))
	{
		BBComp->SetValueAsVector(TEXT("Destination"), InDestination);
	}

	if (BBComp->IsValidKey(BBComp->GetKeyID(TEXT("FailedCheckCount"))))
	{
		int32 FailedCount = 8;
		BBComp->SetValueAsInt(TEXT("FailedCheckCount"), FailedCount);
	}

	if (BTComp->GetCurrentTree())
	{
		if (BTComp->IsRunning())
		{
			BTComp->RestartLogic();
		}
		else
		{
			BTComp->ResumeLogic(TEXT("Resume"));
		}
	}
	else
	{
		BTComp->StartTree(*AutoMoveBehaviorTree);
	}

	IsAutoMoving = true;
}

void AAutoMoveController::StopAutoMove()
{
	UBehaviorTreeComponent* BTComp = GetBehaviorTreeComponent();
	if (!IsValid(BTComp))
	{
		return;
	}

	if (!IsValid(AutoMoveBehaviorTree))
	{
		return;
	}

	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (!IsValid(BBComp))
	{
		return;
	}

	if (BBComp->IsValidKey(BBComp->GetKeyID(TEXT("Destination"))))
	{
		BBComp->SetValueAsVector(TEXT("Destination"), FAISystem::InvalidLocation);
	}

	BTComp->PauseLogic(TEXT("Pause"));

	StopMovement();
	ConnectPawn(nullptr);
	IsAutoMoving = false;
}

bool AAutoMoveController::IsAutoMove()
{
	return IsAutoMoving;
}

bool AAutoMoveController::IsPossibleAutoMove(FVector InDestination)
{
	if (!IsValid(GetPawn()))
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!IsValid(NavSys))
	{
		return false;
	}

	FAIMoveRequest MoveReq;
	MoveReq.SetAcceptanceRadius(0.0f);
	MoveReq.SetNavigationFilter(GetDefaultNavigationFilterClass());
	MoveReq.SetAllowPartialPath(false);
	MoveReq.SetCanStrafe(false);
	MoveReq.SetReachTestIncludesAgentRadius(true);
	MoveReq.SetReachTestIncludesGoalRadius(true);
	MoveReq.SetProjectGoalLocation(true);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetGoalLocation(InDestination);

	FPathFindingQuery PFQuery;
	const bool bValidQuery = BuildPathfindingQuery(MoveReq, PFQuery);

	if (NavSys->TestPathSync(PFQuery))
	{
		return true;
	}

	return false;
}
