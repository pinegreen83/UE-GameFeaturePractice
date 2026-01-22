// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Sandbox/Tickets/Day002-SKILL-STATE-001/ControlStateComponent_SK_ST_001.h"
#include "SkillComponent_SK_ST_001.generated.h"

UENUM(BlueprintType)
enum class ESkillState : uint8
{
	Idle,
	Casting,
	Executing,
	Cooldown
};

UENUM()
enum class ESkillCancelReason : uint8
{
	ByPlayerInput,
	ByHitReact,
	ByCC
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API USkillComponent_SK_ST_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent_SK_ST_001();

	// "스킬 버튼 눌렀다" 진입점
	bool TryStartSkill();

	// 외부(피격/회피 등)에서 "캔슬 요청"
	bool RequestCancel(ESkillCancelReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Skill")
	ESkillState GetSkillState() const { return SkillState; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ESkillState SkillState = ESkillState::Idle;

	UPROPERTY(EditAnywhere, Category = "Skill")
	TObjectPtr<UAnimMontage> SkillMontage = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Skill")
	float CastingDuration = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "Skill")
	float CooldownDuration = 2.5f;

	FTimerHandle CastingTimer;
	FTimerHandle CooldownTimer;
	
	// 내가 Add한 handle만 내가 들고 있다가 Exit에서 제거 (중복 해제 방지)
	FControlBlockHandle MoveLockHandle;
	FControlBlockHandle InputLockHandle;
	
	TWeakObjectPtr<UControlStateComponent_SK_ST_001> ControlComponent;
	
	// 몽타주 종료 콜백 필터링용 : 내가 실행한 몽타주 포인터
	TWeakObjectPtr<UAnimMontage> ActiveMontage;
	
	void TransitionTo(ESkillState NewState);
	
	// State lifecycle
	void EnterState(ESkillState NewState);
	void ExitState(ESkillState OldState);
	
	// Timers
	void OnCastingTimerDone();
	void OnCooldownTimerDone();
	
	// Montage callbacks
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	bool CanCancel(ESkillCancelReason Reason) const;
};
