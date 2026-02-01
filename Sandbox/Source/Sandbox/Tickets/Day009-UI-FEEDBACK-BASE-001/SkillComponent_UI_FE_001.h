// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tickets/Day009-UI-FEEDBACK-BASE-001/SandboxSkillTypes_UI_FE_001.h"
#include "SkillComponent_UI_FE_001.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSandboxSkillUI, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API USkillComponent_UI_FE_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent_UI_FE_001();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Skill")
	void RequestSkillUse(const FSandboxSkillIntent_UI& Intent);
	
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Skill")
	void RequestSkillUseSimple(FName SkillId);

	UPROPERTY(ReplicatedUsing=OnRep_Stamina, BlueprintReadOnly, Category="Sandbox|Skill")
	float Stamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sandbox|Skill")
	float SkillCost = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sandbox|Skill")
	float CooldownSeconds = 1.0f;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_Stamina();

private:
	// === Server-Side ===
	bool bCooldownActive_Server = false;
	FTimerHandle CooldownTimerHandle;

	void Server_SetCooldownActive(bool bActive);
	void Server_BeginCooldown();

	FSandboxSkillResult_UI Server_ValidateAndBuildResult(const FSandboxSkillIntent_UI& Intent) const;
	void Server_ApplySkillImpact(const FSandboxSkillResult_UI& Result);

	// RPCs
	UFUNCTION(Server, Reliable)
	void Server_RequestSkillUse(const FSandboxSkillIntent_UI& Intent);
	
	UFUNCTION(Server, Reliable)
	void Multicast_PlaySkill(const FSandboxSkillResult_UI& Result);

	UFUNCTION(Client, Reliable)
	void Client_RejectSkill(const FSandboxSkillResult_UI& Result);

	// Helpers
	AActor* GetOwnerActor() const;
	APawn* GetOwnerPawn() const;
	APlayerController* GetOwnerPC() const;

	bool IsOwnerLocallyControlled() const;
	
	// === Day009 Feedback ===
	void PublishFeedback_Local(const FSandboxFeedbackEvent& Event) const;
	void PublishFromSkillResult_Local(const FSandboxSkillResult_UI& Result, ESandboxFeedbackResult FeedbackResult, ESandboxFeedbackStage Stage) const;

	// (기존: 연출용)
	void Local_PlaySkillPresentation(const FSandboxSkillResult_UI& Result);
	void Local_PlayRejectPresentation(const FSandboxSkillResult_UI& Result);
};
