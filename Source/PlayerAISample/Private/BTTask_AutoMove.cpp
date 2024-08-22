// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AutoMove.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AutoMoveController.h"
#include "MyPlayerController.h"
#include "GameFrameWork/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"

UBTTask_AutoMove::UBTTask_AutoMove(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Auto Move");
	StopRimitTime = 8.0f;

	bNotifyTaskFinished = true;
	bNotifyTick = true;
	DrawPath = true;
}

EBTNodeResult::Type UBTTask_AutoMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	EBTNodeResult::Type NodeResult = EBTNodeResult::Failed;

	FBTAutoMoveToTaskMemory* MyMemory = CastInstanceNodeMemory<FBTAutoMoveToTaskMemory>(NodeMemory);
	if (!MyMemory)
	{
		return EBTNodeResult::Failed;
	}
	MyMemory->MoveRequestID = FAIRequestID::InvalidRequest;
	MyMemory->StopMoveRimitTime = StopRimitTime;

	AAutoMoveController* AIController = Cast<AAutoMoveController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(AIController->GetPawn());
	if (!IsValid(OwnerCharacter))
	{
		return EBTNodeResult::Failed;
	}

	MyMemory->bWaitingForPath = AIController->ShouldPostponePathUpdates();
	if (!MyMemory->bWaitingForPath)
	{
		NodeResult = PerformMoveTask(OwnerComp, NodeMemory);
	}
	else
	{
		NodeResult = EBTNodeResult::InProgress;
		UE_LOG(LogBehaviorTree, Log, TEXT("Pathfinding requests are freezed, waiting..."));
	}

	return NodeResult;
}

void UBTTask_AutoMove::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
	{
		return;
	}

	AAutoMoveController* AIController = Cast<AAutoMoveController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		return;
	}

	if (TaskResult != EBTNodeResult::Succeeded)
	{
		if (BlackboardComponent->IsValidKey(BlackboardComponent->GetKeyID(TEXT("FailedCheckCount"))))
		{
			int32 FailedCount = BlackboardComponent->GetValueAsInt(TEXT("FailedCheckCount")) - 1;
			if (FailedCount <= 0)
			{
				AIController->StopAutoMove();
				return;
			}
			BlackboardComponent->SetValueAsInt(TEXT("FailedCheckCount"), FailedCount);
		}
	}
	else
	{
		AIController->StopAutoMove();
	}
}

void UBTTask_AutoMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAutoMoveController* AIController = Cast<AAutoMoveController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACharacter* Owner = AIController->GetPawn<ACharacter>();
	if (!IsValid(Owner))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTAutoMoveToTaskMemory* MyMemory = CastInstanceNodeMemory<FBTAutoMoveToTaskMemory>(NodeMemory);
	if (!MyMemory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (MyMemory->MyActor.Get() != Owner)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		AIController->StopAutoMove();
		return;
	}

	if (DrawPath)
	{
		DrawPathSphere(AIController, DeltaSeconds);
	}

	FVector MoveVector = MyMemory->PrevOwnerLocation - Owner->GetActorLocation();
	float MoveAmount = MoveVector.Size();

	if (MoveAmount < 5.0f)
	{
		MyMemory->StopMoveRimitTime -= DeltaSeconds;
		if (MyMemory->StopMoveRimitTime < 0.0f)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}
	else
	{
		MyMemory->StopMoveRimitTime = StopRimitTime;
	}
	MyMemory->PrevOwnerLocation = Owner->GetActorLocation();
}

void UBTTask_AutoMove::DrawPathSphere(AAIController* InAIController, float DeltaSeconds)
{
	ACharacter* Owner = InAIController->GetPawn<ACharacter>();
	if (!IsValid(Owner))
	{
		return;
	}

	// 자동이동 경로 표시
	UPathFollowingComponent* PathFollowingComp = InAIController->GetPathFollowingComponent();
	if (IsValid(PathFollowingComp))
	{
		if (PathFollowingComp->GetPath().IsValid())
		{
			TArray<FNavPathPoint>& PathPoints = PathFollowingComp->GetPath()->GetPathPoints();
			int32 CurrentPathIndex = PathFollowingComp->GetCurrentPathIndex();
			float BetweenDistance = 100.0f;

			for (int32 Path = CurrentPathIndex; Path < PathPoints.Num() - 1; Path++)
			{
				FVector PointA = PathPoints[Path].Location;
				FVector PointB = PathPoints[Path + 1].Location;

				float SegmentDistance = FVector::Dist(PointA, PointB);
				float CheckDistance = Path == CurrentPathIndex ? FVector::Dist(Owner->GetActorLocation(), PointB) : SegmentDistance;
				float AccumulatedDistance = 0.0f;
				while (AccumulatedDistance < CheckDistance)
				{
					FVector CurrentPoint = FMath::Lerp(PointB, PointA, AccumulatedDistance / SegmentDistance);

					DrawDebugSphere(GetWorld(), CurrentPoint, 50.0f, 20, FColor::Green, false, DeltaSeconds);

					AccumulatedDistance += BetweenDistance;
				}
			}
		}

	}
}

uint16 UBTTask_AutoMove::GetInstanceMemorySize() const
{
	return sizeof(FBTAutoMoveToTaskMemory);
}

EBTNodeResult::Type UBTTask_AutoMove::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
	{
		return EBTNodeResult::Failed;
	}

	FBTAutoMoveToTaskMemory* MyMemory = CastInstanceNodeMemory<FBTAutoMoveToTaskMemory>(NodeMemory);
	if (!MyMemory)
	{
		return EBTNodeResult::Failed;
	}

	AAutoMoveController* AIController = Cast<AAutoMoveController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* MyCharacter = AIController->GetPawn<ACharacter>();
	if (!IsValid(MyCharacter))
	{
		return EBTNodeResult::Failed;
	}

	EBTNodeResult::Type NodeResult = EBTNodeResult::Failed;
	if (IsValid(AIController) && IsValid(BlackboardComponent))
	{
		FAIMoveRequest MoveReq;
		MoveReq.SetNavigationFilter(AIController->GetDefaultNavigationFilterClass());
		MoveReq.SetAllowPartialPath(false);
		MoveReq.SetCanStrafe(false);
		MoveReq.SetReachTestIncludesAgentRadius(true);
		MoveReq.SetReachTestIncludesGoalRadius(true);
		MoveReq.SetProjectGoalLocation(true);
		MoveReq.SetAcceptanceRadius(0.0f);
		MoveReq.SetUsePathfinding(true);

		if (BlackboardComponent->IsValidKey(BlackboardComponent->GetKeyID(TEXT("Destination"))))
		{
			FVector Dest = BlackboardComponent->GetValueAsVector(TEXT("Destination"));
			MoveReq.SetGoalLocation(Dest);
		}

		if (MoveReq.IsValid())
		{
			const FPathFollowingRequestResult RequestResult = AIController->MoveTo(MoveReq);
			if (RequestResult.Code == EPathFollowingRequestResult::RequestSuccessful)
			{
				WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished, RequestResult.MoveId);
				WaitForMessage(OwnerComp, UBrainComponent::AIMessage_RepathFailed);
				MyMemory->MyActor = MyCharacter;
				MyMemory->MoveRequestID = RequestResult.MoveId;
				NodeResult = EBTNodeResult::InProgress;

			}
			else if (RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
			{
				NodeResult = EBTNodeResult::Succeeded;
			}
		}
	}

	return NodeResult;
}
