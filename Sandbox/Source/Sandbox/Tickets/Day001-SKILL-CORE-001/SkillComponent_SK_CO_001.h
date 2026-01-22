// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillTypes_SK_CO_001.h"
#include "SkillComponent_SK_CO_001.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API USkillComponent_SK_CO_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent_SK_CO_001();

	// Controller/AI가 호출하는 공통 엔트리
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool RequestActivateSkill(FName SkillId, const FSkillIntentData_SKILL_CORE_001& Intent);

	// UI 확인용
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetCooldownRemaining(FName SkillId) const;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool IsSkillOnCooldown(FName SkillId) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Data ----
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Data")
	TObjectPtr<UDataTable> SkillDataTable;

	// ---- Runtime cooldown state (server authoritative) ----
	// 서버 / 클라 공용 캐시(복제 X) : 빠른 조회용
	TMap<FName, float> CooldownEndTimes_Cache;
	
	UPROPERTY(ReplicatedUsing=OnRep_CooldownEntries)
	TArray<FCooldownEntry_SKILL_CORE_001> CooldownEntries_Replicated;

	UFUNCTION()
	void OnRep_CooldownEntries();
	
	// ---- Server RPC ----
	UFUNCTION(Server, Reliable)
	void ServerTryActivateSkill(FName SkillId, FSkillIntentData_SKILL_CORE_001 Intent);

	// ---- Internal helpers ----
	bool BuildCandidateContext(FSkillExecutionContext_SKILL_CORE_001& OutCtx, const FSkillIntentData_SKILL_CORE_001& Intent) const;
	bool ValidateAndCommit_Server(FName SkillId, const FSkillExecutionContext_SKILL_CORE_001& Ctx, const FSkillDataRow_SKILL_CORE_001& Row);
	void ExecuteSkill_Server(FName SkillId, const FSkillExecutionContext_SKILL_CORE_001& AuthCtx, const FSkillDataRow_SKILL_CORE_001& Row);

	// ---- Cue ----
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlaySkillCue(FSkillCueData_SKILL_CORE_001 Cue);
	
	void PlayCue_Local(const FSkillCueData_SKILL_CORE_001& Cue, const FSkillDataRow_SKILL_CORE_001& Row) const;
	
	// ---- Data lookup ----
	const FSkillDataRow_SKILL_CORE_001* FindSkillRow(FName SkillId) const;
	
	// ---- Debug ----
	void LogSkillEvent(const TCHAR* Event, FName SkillId, const FSkillExecutionContext_SKILL_CORE_001* Ctx = nullptr) const;
};
