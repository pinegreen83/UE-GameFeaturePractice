// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SandboxSkillTypes.h"
#include "SkillComponent_SK_NE_001.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSandboxSkill, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API USkillComponent_SK_NE_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent_SK_NE_001();

	// 클라에서 호출: Intent 만들고 서버 요청
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Skill")
	void RequestSkillUse(const FSandboxSkillIntent& Intent);

	// 테스트 편의: 스킬 ID만 주면 Intent 자동 구성
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Skill")
	void RequestSkillUseSimple(FName SkillId);

	// 서버가 관리하는 자원/상태 (최소 검증용)
	UPROPERTY(ReplicatedUsing=OnRep_Stamina, BlueprintReadOnly, Category = "Sandbox|Skill")
	float Stamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sandbox|Skill")
	float SkillCost = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sandbox|Skill")
	float CooldownSeconds = 1.0f;
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Stamina();

private:
	// === Server-side ===
	bool bCooldownActive_Server = false;
	FTimerHandle CooldownTimerHandle;

	void Server_SetCooldownActive(bool bActive);
	void Server_BeginCooldown();

	FSandboxSkillResult Server_ValidateAndBuildResult(const FSandboxSkillIntent& Intent) const;
	void Server_ApplySkillImpact(const FSandboxSkillResult& Result);

	// RPCs
	UFUNCTION(Server, Reliable)
	void Server_RequestSkillUse(const FSandboxSkillIntent& Intent);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkill(const FSandboxSkillResult& Result);

	UFUNCTION(Client, Reliable)
	void Client_RejectSkill(const FSandboxSkillResult& Result);

	// Helpers
	AActor* GetOwnerActor() const;
	APawn* GetOwnerPawn() const;
	APlayerController* GetOwnerPC() const;

	void Local_PlaySkillPresentation(const FSandboxSkillResult& Result);
	void Local_PlayRejectPresentation(const FSandboxSkillResult& Result);
};
