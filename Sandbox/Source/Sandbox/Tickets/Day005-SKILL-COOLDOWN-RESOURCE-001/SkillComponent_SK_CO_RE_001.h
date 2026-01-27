// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillTypes_SK_CO_RE.h"
#include "SkillComponent_SK_CO_RE_001.generated.h"

class UStatComponent_SK_CO_RE_001;

DECLARE_LOG_CATEGORY_EXTERN(LogSandboxSkill_CO, Log, All);

USTRUCT()
struct FSkillContext
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* OwnerActor = nullptr;

	UPROPERTY()
	UStatComponent_SK_CO_RE_001* StatComponent = nullptr;

	// 정의는 캐시된 값(수정 금지)
	const FSkillDef* SkillDef = nullptr;

	// Resolve/Validate 단계에서는 읽기만 -> const 포인터
	const FSkillRuntimeState* RuntimeState = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API USkillComponent_SK_CO_RE_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent_SK_CO_RE_001();

	// === 단일 진입점 ===
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Skill")
	void RequestActivateSkill(const FName SkillId, const FSkillIntent& Intent);

protected:
	virtual void BeginPlay() override;

	// --- Server RPC entry ---
	UFUNCTION(Server, Reliable)
	void Server_TryActivateSkill(const FName SkillId, const FSkillIntent& Intent);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSkillResult(const FName SkillId, const FSkillResult& Result, const FSkillIntent& Intent);

	UFUNCTION(Client, Reliable)
	void Client_OnSkillResult(const FName SkillId, const FSkillResult& Result);

private:
	// === 서버 내부 순수 로직: RPC 호출 절대 금지 ===
	bool TryActivateSkill_InternalServer(const FName SkillId, const FSkillIntent& Intent, FSkillResult& OutResult);
	
	bool ResolveContext(const FName SkillId, FSkillContext& OutContext, FSkillResult& OutFailure) const;
	FSkillValidationResult Validate(const FSkillContext& Context, const FSkillIntent& Intent) const;
	void Commit(const FSkillContext& Context, const FSkillIntent& Intent);
	void Execute(const FSkillContext& Context, const FSkillIntent& Intent);

	// === Data ===
	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|Skill")
	UDataTable* SkillDataTable = nullptr;

	// SkillId -> Def 캐시(가벼운 수치)
	UPROPERTY(Transient)
	TMap<FName, FSkillDef> SkillDefCache;

	// SkillId -> RuntimeState(캐릭터 별)
	UPROPERTY(Transient)
	TMap<FName, FSkillRuntimeState> RuntimeStateMap;

	// Dependency cache
	UPROPERTY(Transient)
	TWeakObjectPtr<UStatComponent_SK_CO_RE_001> StatComponentCached;

	float GetServerNow() const;
	void BuildCacheFromDataTable();
};
