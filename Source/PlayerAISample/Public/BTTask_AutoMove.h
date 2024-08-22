// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AutoMove.generated.h"

class AAIController;

struct FBTAutoMoveToTaskMemory
{
	FAIRequestID MoveRequestID;
	TWeakObjectPtr<AActor> MyActor;
	FVector PrevOwnerLocation = FVector::ZeroVector;

	uint8 bWaitingForPath : 1;

	float StopMoveRimitTime = 0.0f;
};

UCLASS()
class PLAYERAISAMPLE_API UBTTask_AutoMove : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_AutoMove(const FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void DrawPathSphere(AAIController* InAIController, float DeltaSeconds);

	virtual uint16 GetInstanceMemorySize() const override;
private:
	EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

private:
	// 한 자리에 n초 이상 머무르면 AutoMove 종료
	UPROPERTY(EditAnywhere, Category = "Auto Move", DisplayName = "Stop Rimit Time")
	float StopRimitTime;

	UPROPERTY(EditAnywhere, Category = "Auto Move", DisplayName = "Draw Path")
	bool DrawPath;

};
