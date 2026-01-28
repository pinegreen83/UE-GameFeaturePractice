// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day006-PLAYER-STATE-LOCK-001/InputGateComponent_PL_ST_001.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/StatusComponent_PL_ST_001.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/InputConsumerInterface_PL_ST_001.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogInputGateComp, Log, All);

UInputGateComponent_PL_ST_001::UInputGateComponent_PL_ST_001()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInputGateComponent_PL_ST_001::SetControlledPawn(APawn* NewPawn)
{
	ControlledPawn = NewPawn;
}

EInputRejectReason UInputGateComponent_PL_ST_001::CheckCommonPolicy() const
{
	APawn* Pawn = ControlledPawn.Get();
	if (!Pawn) return EInputRejectReason::MissingPawn;

	const UStatusComponent_PL_ST_001* StatusComponent = Pawn->FindComponentByClass<UStatusComponent_PL_ST_001>();
	if (!StatusComponent) return EInputRejectReason::MissingStatus;

	if (StatusComponent->IsStunned()) return EInputRejectReason::Stunned;

	return EInputRejectReason::None;
}

void UInputGateComponent_PL_ST_001::RequestMoveAxis2D(const FVector2D& Value)
{
	const EInputRejectReason Reason = CheckCommonPolicy();
	if (Reason != EInputRejectReason::None)
	{
		OnInputRejected.Broadcast(TEXT("Move"), Reason);
		return;
	}

	APawn* Pawn = ControlledPawn.Get();
	if (!Pawn) return;

	if (IInputConsumerInterface_PL_ST_001* Consumer = Cast<IInputConsumerInterface_PL_ST_001>(Pawn))
	{
		Consumer->ConsumeAxis2D_Move(Value);
	}
}

void UInputGateComponent_PL_ST_001::RequestAttackPressed()
{
	const EInputRejectReason Reason = CheckCommonPolicy();
	if (Reason != EInputRejectReason::None)
	{
		UE_LOG(LogInputGateComp, Warning, TEXT("RequestAttackPressed is Stun? %d"), Reason);
		OnInputRejected.Broadcast(TEXT("AttackPressed"), Reason);
		return;
	}

	APawn* Pawn = ControlledPawn.Get();
	if(!Pawn) return;

	if (IInputConsumerInterface_PL_ST_001* Consumer = Cast<IInputConsumerInterface_PL_ST_001>(Pawn))
	{
		Consumer->ConsumeTrigger_Attack_Pressed();
	}
}

void UInputGateComponent_PL_ST_001::RequestAttackReleased()
{
	const EInputRejectReason Reason = CheckCommonPolicy();
	if (Reason != EInputRejectReason::None)
	{
		OnInputRejected.Broadcast(TEXT("AttackReleased"), Reason);
		return;
	}

	APawn* Pawn = ControlledPawn.Get();
	if(!Pawn) return;

	if (IInputConsumerInterface_PL_ST_001* Consumer = Cast<IInputConsumerInterface_PL_ST_001>(Pawn))
	{
		Consumer->ConsumeTrigger_Attack_Released();
	}
}
