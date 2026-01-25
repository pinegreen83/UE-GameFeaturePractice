// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day004-SKILL-NET-AUTH-001/SkillComponent_SK_NE_001.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogSandboxSkill)

static FString GetNetContextTag(const UObject* Context)
{
	const AActor* OwnerActor = Context ? Context->GetTypedOuter<AActor>() : nullptr;

	const UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	const ENetMode NetMode = World ? World->GetNetMode() : NM_Standalone;

	auto NetModeToStr = [](ENetMode M)
	{
		switch (M)
		{
		case NM_Standalone:     return TEXT("Standalone");
		case NM_ListenServer:   return TEXT("ListenServer");
		case NM_DedicatedServer:return TEXT("DedicatedServer");
		case NM_Client:         return TEXT("Client");
		default:                return TEXT("UnknownNetMode");
		}
	};

	const FString ActorName = OwnerActor ? OwnerActor->GetName() : TEXT("UnknownActor");
	const FString RoleStr = OwnerActor ? UEnum::GetValueAsString(OwnerActor->GetLocalRole()) : TEXT("NoRole");
	FString CleanRole = RoleStr;
	CleanRole.RemoveFromStart(TEXT("ENetRole::"));
	CleanRole.RemoveFromStart(TEXT("ROLE_"));

	const float T = World ? World->GetTimeSeconds() : -1.f;

	return FString::Printf(TEXT("[T=%.3f][%s][%s|%s]"),
		T, NetModeToStr(NetMode), *ActorName, *CleanRole);
}

// Sets default values for this component's properties
USkillComponent_SK_NE_001::USkillComponent_SK_NE_001()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void USkillComponent_SK_NE_001::RequestSkillUse(const FSandboxSkillIntent& Intent)
{
	Server_RequestSkillUse(Intent);
}

void USkillComponent_SK_NE_001::RequestSkillUseSimple(FName SkillId)
{
	FSandboxSkillIntent Intent;
	Intent.SkillId = SkillId;

	if (AActor* OwnerActor = GetOwnerActor())
	{
		Intent.ClientOrigin = OwnerActor->GetActorLocation();
		Intent.ClientForward = OwnerActor->GetActorForwardVector();
	}

	RequestSkillUse(Intent);
}

void USkillComponent_SK_NE_001::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USkillComponent_SK_NE_001, Stamina);
}

void USkillComponent_SK_NE_001::OnRep_Stamina()
{
	// UI 연동이 필요하면 여기서 처리.
}

void USkillComponent_SK_NE_001::Server_SetCooldownActive(bool bActive)
{
	bCooldownActive_Server = bActive;
}

void USkillComponent_SK_NE_001::Server_BeginCooldown()
{
	Server_SetCooldownActive(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			FTimerDelegate::CreateUObject(this, &USkillComponent_SK_NE_001::Server_SetCooldownActive, false),
			CooldownSeconds,
			false
		);
	}
}

FSandboxSkillResult USkillComponent_SK_NE_001::Server_ValidateAndBuildResult(const FSandboxSkillIntent& Intent) const
{
	FSandboxSkillResult Result;
	Result.SkillId = Intent.SkillId;
	Result.PredictionKey = Intent.PredictionKey;

	// 서버가 확정할 값(권장: 서버 기준으로 재확정)
	if (const AActor* OwnerActor = GetOwnerActor())
	{
		Result.ServerOrigin = OwnerActor->GetActorLocation();
		Result.ServerForward = OwnerActor->GetActorForwardVector();
	}

	// 0) SkillId 기본 검증
	if (Intent.SkillId.IsNone())
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason::InvalidData;
		return Result;
	}

	// 1) Owner 검증: 요청자가 이 컴포넌트를 가진 Pawn의 Owner인지(간단 검증)
	// UE RPC 자체가 기본적으로 "해당 액터/컴포넌트 소유자"에서 오는 걸 기대하지만,
	// 여기서는 명시적으로 한번 더 의도 표현.
	if (APawn* Pawn = GetOwnerPawn())
	{
		if (!Pawn->GetController())
		{
			Result.bApproved = false;
			Result.RejectReason = ESandboxSkillRejectReason::NotOwner;
			return Result;
		}
	}

	// 2) 상태 검증(확장 포인트)
	// 예: 죽음/경직/채널링 등.
	// Result.bApproved = false;
	// Result.RejectReason = ESandboxSkillRejectReason::StateBlocked;

	// 3) 쿨타임
	if (bCooldownActive_Server)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason::Cooldown;
		return Result;
	}

	// 4) 자원(스태미나)
	if (Stamina < SkillCost)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason::NotEnoughStamina;
		return Result;
	}

	Result.bApproved = true;
	Result.RejectReason = ESandboxSkillRejectReason::None;
	return Result;
}

void USkillComponent_SK_NE_001::Server_ApplySkillImpact(const FSandboxSkillResult& Result)
{
	// === 서버만 월드 상태 변경 ===
	Stamina = FMath::Max(0.f, Stamina - SkillCost);
	Server_BeginCooldown();
	
	// 여기서 데미지/상태이상/투사체 스폰 등 실제 영향 적용을 확장
	UE_LOG(LogSandboxSkill, Log, TEXT("%s Approved Skill=%s Stamina=%.1f"), *GetNetContextTag(this), *Result.SkillId.ToString(), Stamina);
}

void USkillComponent_SK_NE_001::Server_RequestSkillUse_Implementation(const FSandboxSkillIntent& Intent)
{
	// === 서버에서만 판정/결정 ===
	FSandboxSkillResult Result = Server_ValidateAndBuildResult(Intent);

	if (!Result.bApproved)
	{
		Client_RejectSkill(Result);
		return;
	}

	Server_ApplySkillImpact(Result);
	Multicast_PlaySkill(Result);
}

void USkillComponent_SK_NE_001::Multicast_PlaySkill_Implementation(const FSandboxSkillResult& Result)
{
	// 모든 클라 + 서버에서 호출됨 (서버도 포함)
	Local_PlaySkillPresentation(Result);
}

void USkillComponent_SK_NE_001::Client_RejectSkill_Implementation(const FSandboxSkillResult& Result)
{
	// 요청한 클라(Owner)만 받음
	Local_PlayRejectPresentation(Result);
}

AActor* USkillComponent_SK_NE_001::GetOwnerActor() const
{
	return GetOwner();
}

APawn* USkillComponent_SK_NE_001::GetOwnerPawn() const
{
	return Cast<APawn>(GetOwner());
}

APlayerController* USkillComponent_SK_NE_001::GetOwnerPC() const
{
	if (APawn* Pawn = GetOwnerPawn())
	{
		return Cast<APlayerController>(Pawn->GetController());
	}
	return nullptr;
}

void USkillComponent_SK_NE_001::Local_PlaySkillPresentation(const FSandboxSkillResult& Result)
{
	// "연출"만: 지금은 디버그 메시지로 확인
	const FString Msg = FString::Printf(TEXT("Play SKill: %s (Origin=%s)"),
		*Result.SkillId.ToString(), *Result.ServerOrigin.ToString());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::Green, Msg);
	}
	
	UE_LOG(LogSandboxSkill,Warning, TEXT("%s %s"), *GetNetContextTag(this), *Msg);
}

void USkillComponent_SK_NE_001::Local_PlayRejectPresentation(const FSandboxSkillResult& Result)
{
	FString ReasonStr = UEnum::GetValueAsString(Result.RejectReason);
	ReasonStr.RemoveFromStart(TEXT("ESandboxSkillRejectReason::"));

	const FString Msg = FString::Printf(TEXT("Reject Skill: %s (Reason=%s)"),
		*Result.SkillId.ToString(), *ReasonStr);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::Red, Msg);
	}

	UE_LOG(LogSandboxSkill,Warning, TEXT("%s %s"), *GetNetContextTag(this), *Msg);
}
