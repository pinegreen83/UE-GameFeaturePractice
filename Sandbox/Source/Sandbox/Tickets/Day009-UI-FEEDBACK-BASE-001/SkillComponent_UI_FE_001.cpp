// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day009-UI-FEEDBACK-BASE-001/SkillComponent_UI_FE_001.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Tickets/Day009-UI-FEEDBACK-BASE-001/FeedbackSubsystem_UI_FE_001.h"

DEFINE_LOG_CATEGORY(LogSandboxSkillUI)

// Sets default values for this component's properties
USkillComponent_UI_FE_001::USkillComponent_UI_FE_001()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void USkillComponent_UI_FE_001::RequestSkillUse(const FSandboxSkillIntent_UI& Intent)
{
	Server_RequestSkillUse(Intent);
}

void USkillComponent_UI_FE_001::RequestSkillUseSimple(FName SkillId)
{
	UE_LOG(LogTemp, Log, TEXT("RequestSkillUse"));
	
	FSandboxSkillIntent_UI Intent;
	Intent.SkillId = SkillId;

	if (AActor* OwnerActor = GetOwnerActor())
	{
		Intent.ClientOrigin = OwnerActor->GetActorLocation();
		Intent.ClientForward = OwnerActor->GetActorForwardVector();
	}
	
	RequestSkillUse(Intent);
}

bool USkillComponent_UI_FE_001::IsOwnerLocallyControlled() const
{
	if (const APawn* Pawn = GetOwnerPawn())
	{
		return Pawn->IsLocallyControlled();
	}
	return false;
}

void USkillComponent_UI_FE_001::PublishFeedback_Local(const FSandboxFeedbackEvent& Event) const
{
	if (!GetWorld()) return;
	if(UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		if(UFeedbackSubsystem_UI_FE_001* Subsystem = GameInstance->GetSubsystem<UFeedbackSubsystem_UI_FE_001>())
		{
			Subsystem->Publish(Event);
		}
	}
}

void USkillComponent_UI_FE_001::PublishFromSkillResult_Local(const FSandboxSkillResult_UI& Result,
	ESandboxFeedbackResult FeedbackResult, ESandboxFeedbackStage Stage) const
{
	FSandboxFeedbackEvent Event;
	Event.Result = FeedbackResult;
	Event.Reason = (FeedbackResult == ESandboxFeedbackResult::Fail) ? Result.RejectReason : ESandboxSkillRejectReason_UI::None;
	Event.Stage = Stage;
	Event.SkillId = Result.SkillId;
	
	PublishFeedback_Local(Event);
}

FSandboxSkillResult_UI USkillComponent_UI_FE_001::Server_ValidateAndBuildResult(
	const FSandboxSkillIntent_UI& Intent) const
{
	FSandboxSkillResult_UI Result;
	Result.SkillId = Intent.SkillId;
	Result.PredictionKey = Intent.PredictionKey;

	// 서버 확정 값
	if (const AActor* OwnerActor =GetOwnerActor())
	{
		Result.ServerOrigin = OwnerActor->GetActorLocation();
		Result.ServerForward = OwnerActor->GetActorForwardVector();
	}

	// 0) SkillId
	if(Intent.SkillId.IsNone())
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason_UI::InvalidData;
		return Result;
	}

	// 1) Owner 검증(간단)
	if (APawn* Pawn = GetOwnerPawn())
	{
		if (!Pawn->GetController())
		{
			Result.bApproved = false;
			Result.RejectReason = ESandboxSkillRejectReason_UI::NotOwner;
			return Result;
		}
	}

	// 2) 상태 검증(확장 포인트)
	// Result.bApproved = false;
	// Result.RejectReason = ESandboxSkillRejectReason::StateBlocked;

	// 3) 쿨타임
	if(bCooldownActive_Server)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason_UI::Cooldown;
		return Result;
	}

	// 4) 자원
	if(Stamina < SkillCost)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason_UI::NotEnoughStamina;
		return Result;
	}

	Result.bApproved = true;
	Result.RejectReason = ESandboxSkillRejectReason_UI::None;
	return Result;
}

void USkillComponent_UI_FE_001::Server_RequestSkillUse_Implementation(const FSandboxSkillIntent_UI& Intent)
{
	FSandboxSkillResult_UI Result = Server_ValidateAndBuildResult(Intent);

	if (!Result.bApproved)
	{
		// 실패: Owner에게만
		Client_RejectSkill(Result);
		return;
	}
	
	// Commit(서버 월드 상태 변경)
	Server_ApplySkillImpact(Result);
	
	// 성공 연출은 기존대로 multicast(다른 플레이어도 보게)
	Multicast_PlaySkill(Result);
}

void USkillComponent_UI_FE_001::Multicast_PlaySkill_Implementation(const FSandboxSkillResult_UI& Result)
{
	Local_PlaySkillPresentation(Result);
	
	// 성공 피드백은 "요청한 본인"에게만 뜨게
	if (IsOwnerLocallyControlled())
	{
		PublishFromSkillResult_Local(Result, ESandboxFeedbackResult::Success, ESandboxFeedbackStage::Commit);
	}
}

void USkillComponent_UI_FE_001::Client_RejectSkill_Implementation(const FSandboxSkillResult_UI& Result)
{
	Local_PlayRejectPresentation(Result);
	
	// 실패 피드백은 즉시, Owner 전용 (Client RPC니까 여긴 이미 Owner)
	PublishFromSkillResult_Local(Result, ESandboxFeedbackResult::Fail, ESandboxFeedbackStage::Validate);
}

void USkillComponent_UI_FE_001::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillComponent_UI_FE_001, Stamina);
}

void USkillComponent_UI_FE_001::OnRep_Stamina()
{
	// UI 연동이 필요하면 여기서 처리.
}

void USkillComponent_UI_FE_001::Server_SetCooldownActive(bool bActive)
{
	bCooldownActive_Server = bActive;
}

void USkillComponent_UI_FE_001::Server_BeginCooldown()
{
	Server_SetCooldownActive(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			FTimerDelegate::CreateUObject(this, &USkillComponent_UI_FE_001::Server_SetCooldownActive, false),
			CooldownSeconds,
			false
		);
	}
}

void USkillComponent_UI_FE_001::Server_ApplySkillImpact(const FSandboxSkillResult_UI& Result)
{
	// === 서버만 월드 상태 변경 ===
	Stamina = FMath::Max(0.f, Stamina - SkillCost);
	Server_BeginCooldown();
}

AActor* USkillComponent_UI_FE_001::GetOwnerActor() const
{
	return GetOwner();
}

APawn* USkillComponent_UI_FE_001::GetOwnerPawn() const
{
	return Cast<APawn>(GetOwner());
}

APlayerController* USkillComponent_UI_FE_001::GetOwnerPC() const
{
	if (APawn* Pawn = GetOwnerPawn())
	{
		return Cast<APlayerController>(Pawn->GetController());
	}

	return nullptr;
}

void USkillComponent_UI_FE_001::Local_PlaySkillPresentation(const FSandboxSkillResult_UI& Result)
{
}

void USkillComponent_UI_FE_001::Local_PlayRejectPresentation(const FSandboxSkillResult_UI& Result)
{
}
