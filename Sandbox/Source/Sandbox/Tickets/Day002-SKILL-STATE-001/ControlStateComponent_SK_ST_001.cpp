// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day002-SKILL-STATE-001/ControlStateComponent_SK_ST_001.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

DEFINE_LOG_CATEGORY_STATIC(LogControlState, Log, All);

UControlStateComponent_SK_ST_001::UControlStateComponent_SK_ST_001()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UControlStateComponent_SK_ST_001::BeginPlay()
{
	Super::BeginPlay();
	RecomputeAndApply();
}

FControlBlockHandle UControlStateComponent_SK_ST_001::AddBlockReason(EControlReason Reason, const TCHAR* DebugTea)
{
	const int32 Id = NextHandleId++;
	LiveHandles.Add(Id, Reason);
	
	int32& Count = ReasonCounts.FindOrAdd(Reason);
	Count++;

	UE_LOG(LogControlState, Log, TEXT("[AddReason] Handle=%d Reason=%d CountNow=%d"),
		Id, (int32)Reason, ReasonCounts.FindRef(Reason));
	
	RecomputeAndApply();
	
	FControlBlockHandle Handle;
	Handle.Id = Id;
	Handle.Reason = Reason;
	return Handle;
}

void UControlStateComponent_SK_ST_001::RemoveBlockReason(const FControlBlockHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}
	
	EControlReason* FoundReason = LiveHandles.Find(Handle.Id);
	if (!FoundReason)
	{
		return;
	}
	
	// 카운드 감소
	if (int32* Count = ReasonCounts.Find(*FoundReason))
	{
		*Count = FMath::Max(0, *Count - 1);
	}
	LiveHandles.Remove(Handle.Id);
	
	UE_LOG(LogControlState, Log, TEXT("[RemoveReason] Handle=%d Reason=%d CountNow=%d"),
		Handle.Id, (int32)Handle.Reason, ReasonCounts.FindRef(Handle.Reason));
	
	RecomputeAndApply();
}

void UControlStateComponent_SK_ST_001::SetOverrideState(EControlOverrideState NewState)
{
	if (OverrideState == NewState)
	{
		return;
	}
	
	UE_LOG(LogControlState, Log, TEXT("[Override] %d -> %d"), (int32)OverrideState, (int32)NewState);
	
	OverrideState = NewState;
	RecomputeAndApply();
}

void UControlStateComponent_SK_ST_001::RecomputeAndApply()
{
	FControlSnapshot NewSnapshot;
	NewSnapshot.OverrideState = OverrideState;

	// 1) Override가 있으면 강제 덮어쓰기
	if (OverrideState != EControlOverrideState::None)
	{
		NewSnapshot.bCanMove = false;
		NewSnapshot.bCanInput = false;
		NewSnapshot.bCanLook = false;
		Snapshot = NewSnapshot;
		ApplyToPawn(Snapshot);
		return;
	}

	// 2) 없으면 Reason 핪엉으로 계산
	const int32 MoveLockCount = ReasonCounts.FindRef(EControlReason::MoveLock);
	const int32 InputLockCount = ReasonCounts.FindRef(EControlReason::InputLock);
	const int32 LockLockCount = ReasonCounts.FindRef(EControlReason::LookLock);
	
	NewSnapshot.bCanMove = (MoveLockCount <= 0);
	NewSnapshot.bCanInput = (InputLockCount <= 0);
	NewSnapshot.bCanLook = (LockLockCount <= 0);

	// 상태 변경시에만 Apply하고 싶으면 여기서 diff 체크
	const bool bMoveBecameLocked = Snapshot.bCanMove && !NewSnapshot.bCanMove;

	UE_LOG(LogControlState, Log, TEXT("[Snapshot] Override=%d CanMove=%d CanInput=%d CanLook=%d (MoveLock=%d InputLock=%d LookLock=%d)"),
		(int32)NewSnapshot.OverrideState,
		NewSnapshot.bCanMove ? 1 : 0,
		NewSnapshot.bCanInput ? 1 : 0,
		NewSnapshot.bCanLook ? 1 : 0,
		ReasonCounts.FindRef(EControlReason::MoveLock),
		ReasonCounts.FindRef(EControlReason::InputLock),
		ReasonCounts.FindRef(EControlReason::LookLock));

	Snapshot = NewSnapshot;
	ApplyToPawn(Snapshot);

	// MoveLock이 막히는 순간 관성/미끄러짐 방지: 1회 정지
	if (bMoveBecameLocked)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				MovementComponent->StopMovementImmediately();
			}
		}	
	}
	
}

void UControlStateComponent_SK_ST_001::ApplyToPawn(const FControlSnapshot& NewSnapshot)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}
	
	AController* Controller = Character->GetController();
	if (!Controller)
	{
		return;
	}

	UE_LOG(LogControlState, Log, TEXT("[Apply] IgnoreMove=%d IgnoreLook=%d"),
		NewSnapshot.bCanMove ? 0 : 1,
		NewSnapshot.bCanLook ? 0 : 1);
	
	// 실제 "막아라/풀어라"는 여기서만 실행됨.
	Controller->SetIgnoreMoveInput(!NewSnapshot.bCanMove);
	Controller->SetIgnoreLookInput(!NewSnapshot.bCanLook);

	// bCanInput은 프로젝트마다 적용 방식이 다름:
	// - Enhanced Input Mapping Context 제거/추가
	// - 또는 Controller에서 입력 이벤트 무시
}

