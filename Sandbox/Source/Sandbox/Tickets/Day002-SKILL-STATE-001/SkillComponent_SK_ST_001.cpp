// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day002-SKILL-STATE-001/SkillComponent_SK_ST_001.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkillState, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogControlState, Log, All);

USkillComponent_SK_ST_001::USkillComponent_SK_ST_001()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent_SK_ST_001::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		ControlComponent = Owner->FindComponentByClass<UControlStateComponent_SK_ST_001>();
	}
	
	// 시작 상태
	TransitionTo(ESkillState::Idle);
}

bool USkillComponent_SK_ST_001::TryStartSkill()
{
	if (SkillState != ESkillState::Idle)
	{
		return false;
	}

	UE_LOG(LogSkillState, Log, TEXT("[TryStartSkill] State=%d"), (int32)SkillState);
	
	TransitionTo(ESkillState::Casting);
	return true;
}

bool USkillComponent_SK_ST_001::RequestCancel(ESkillCancelReason Reason)
{
	if (!CanCancel(Reason))
	{
		return false;
	}
	
	UE_LOG(LogSkillState, Log, TEXT("[RequestCancel] Reason=%d State=%d CanCancel=%s"),
	(int32)Reason, (int32)SkillState, CanCancel(Reason) ? TEXT("YES") : TEXT("NO"));
	
	// 캔슬되면 Cooldown으로 보낼지 Idle로 보낼지는 상황마다 다름. 우선은 Cooldown으로
	TransitionTo(ESkillState::Cooldown);
	return true;
}

void USkillComponent_SK_ST_001::TransitionTo(ESkillState NewState)
{
	if(SkillState == NewState)
	{
		return;
	}

	UE_LOG(LogSkillState, Log, TEXT("[Transition] %d -> %d"), (int32)SkillState, (int32)NewState);
	
	const ESkillState Old = SkillState;
	ExitState(Old);
	SkillState = NewState;
	EnterState(NewState);
}

void USkillComponent_SK_ST_001::EnterState(ESkillState NewState)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if(!Character)
	{
		return;
	}

	UE_LOG(LogSkillState, Log, TEXT("[Enter] State=%d"), (int32)NewState);

	switch (NewState)
	{
		case ESkillState::Idle:
			// 아무 제한 없음(Exit에서 해제가 보장됨)
			break;
		
		case ESkillState::Casting:
			// 채널링 / 시전 동안 이동 막기 예시:
			if (ControlComponent.IsValid() && !MoveLockHandle.IsValid())
			{
				MoveLockHandle = ControlComponent->AddBlockReason(EControlReason::MoveLock, TEXT("Skill Casting"));
			}

			// 필요하면 입력도 잠글 수 있음(예시)
			if (ControlComponent.IsValid() && !InputLockHandle.IsValid())
			{
					InputLockHandle = ControlComponent->AddBlockReason(EControlReason::InputLock, TEXT("Skill Casting"));
			}
			
			GetWorld()->GetTimerManager().SetTimer(CastingTimer, this, &USkillComponent_SK_ST_001::OnCastingTimerDone, CastingDuration, false);
			break;

		case ESkillState::Executing:
			{
				// 몽타주 재생 + 종료 델리게이트 바인딩(종료 보장)
				UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
				if(AnimInstance && SkillMontage)
				{
					ActiveMontage = SkillMontage;

					//	종료 / 블렌딩 아웃 콜백 바인딩
					AnimInstance->OnMontageEnded.AddDynamic(this, &USkillComponent_SK_ST_001::OnMontageEnded);
					AnimInstance->OnMontageBlendingOut.AddDynamic(this, &USkillComponent_SK_ST_001::OnMontageBlendingOut);
			
					AnimInstance->Montage_Play(SkillMontage);
				}
				else
				{
					// 애니메이션 없으면 즉시 Cooldown으로
					TransitionTo(ESkillState::Cooldown);
				}
			}
			break;
		case ESkillState::Cooldown:
			// 보통 쿨다운은 이동/입력 허용. 여기선 즉시 해제되도록(Exit에서 해제되도록 Enter에서 할 것 없음)
			GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &USkillComponent_SK_ST_001::OnCooldownTimerDone, CooldownDuration, false);
			break;
	}
}

void USkillComponent_SK_ST_001::ExitState(ESkillState OldState)
{
	UE_LOG(LogSkillState, Log, TEXT("[Exit] State=%d MoveHandle=%d InputHandle=%d"),
		(int32)OldState, MoveLockHandle.Id, InputLockHandle.Id);
	
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	switch(OldState)
	{
		case ESkillState::Casting:
			GetWorld()->GetTimerManager().ClearTimer(CastingTimer);
			break;

		case ESkillState::Executing:
			// 델리게이트 언바인딩 + 몽타주 정리
			if(Character && Character->GetMesh())
			{
				if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
				{
					AnimInstance->OnMontageEnded.RemoveDynamic(this, &USkillComponent_SK_ST_001::OnMontageEnded);
					AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &USkillComponent_SK_ST_001::OnMontageBlendingOut);
				}
			}
			ActiveMontage.Reset();
			break;
			
		case ESkillState::Cooldown:
			GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
			break;

		default:
			break;
	}
	
	// ***핵심 : 어떤 이유로 전이되든 여기서 해제(Exit으로 수렴)***
	if(ControlComponent.IsValid())
	{
		if(MoveLockHandle.IsValid())
		{
			ControlComponent->RemoveBlockReason(MoveLockHandle);
			MoveLockHandle = FControlBlockHandle{};
		}
		if (InputLockHandle.IsValid())
		{
			ControlComponent->RemoveBlockReason(InputLockHandle);
			InputLockHandle = FControlBlockHandle{};
		}
	}
}

void USkillComponent_SK_ST_001::OnCastingTimerDone()
{
	// Casting 끝 -> Executing
	if (SkillState == ESkillState::Casting)
	{
		TransitionTo(ESkillState::Executing);
	}
}

void USkillComponent_SK_ST_001::OnCooldownTimerDone()
{
	if (SkillState == ESkillState::Cooldown)
	{
		TransitionTo(ESkillState::Idle);
	}
}

void USkillComponent_SK_ST_001::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// BlendingOut에서 이미 처리했을 수 있으니, 같은 몽타주라도 상태 체크로 중복 방지
	if (!ActiveMontage.IsValid() || Montage != ActiveMontage.Get())
	{
		return;
	}

	UE_LOG(LogSkillState, Log, TEXT("[MontageBlendingOut] State=%d Montage=%s Active=%s Interrupted=%d"),
    	(int32)SkillState,
    	*GetNameSafe(Montage),
    	*GetNameSafe(ActiveMontage.Get()),
    	bInterrupted ? 1 : 0);

	if (SkillState == ESkillState::Executing)
	{
		TransitionTo(ESkillState::Cooldown);
	}
}

void USkillComponent_SK_ST_001::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// "내가 실행한 몽타주"만 처리
	if(!ActiveMontage.IsValid() || Montage != ActiveMontage.Get())
	{
		return;
	}

	UE_LOG(LogSkillState, Log, TEXT("[MontageBlendingOut] State=%d Montage=%s Active=%s Interrupted=%d"),
		(int32)SkillState,
		*GetNameSafe(Montage),
		*GetNameSafe(ActiveMontage.Get()),
		bInterrupted ? 1 : 0);
	
	// Executing 중이면 Cooldown으로
	if(SkillState == ESkillState::Executing)
	{
		TransitionTo(ESkillState::Cooldown);
	}
}

bool USkillComponent_SK_ST_001::CanCancel(ESkillCancelReason Reason) const
{
	// 예시 정책:
	// - Idle은 캔슬 불필요
	// - Casting / Executing은 ByPlayerInput / ByHitReact / ByCC 모두 캔슬 가능.
	// - Cooldown은 캔슬 없음.
	switch (SkillState)
	{
		case ESkillState::Casting:
		case ESkillState::Executing:
			return true;
		default:
			return false;
	}
}
