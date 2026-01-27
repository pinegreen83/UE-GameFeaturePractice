// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day005-SKILL-COOLDOWN-RESOURCE-001/SkillComponent_SK_CO_RE_001.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "StatComponent_SK_CO_RE_001.h"

DEFINE_LOG_CATEGORY(LogSandboxSkill_CO)

static FString SK_NetTag(const UObject* ContextObj)
{
	const AActor* Owner = nullptr;

	if (const UActorComponent* Comp = Cast<UActorComponent>(ContextObj))
	{
		Owner = Comp->GetOwner();
	}
	else
	{
		Owner = Cast<AActor>(ContextObj);
	}

	const ENetMode NetMode = Owner ? Owner->GetNetMode() : NM_Standalone;
	const FString NetModeStr =
		(NetMode == NM_Client) ? TEXT("Client") :
		(NetMode == NM_ListenServer) ? TEXT("ListenServer") :
		(NetMode == NM_DedicatedServer) ? TEXT("DedicatedServer") :
		TEXT("Standalone");

	const ENetRole LocalRole = Owner ? Owner->GetLocalRole() : ROLE_None;
	const ENetRole RemoteRole = Owner ? Owner->GetRemoteRole() : ROLE_None;

	const FString OwnerName = Owner ? Owner->GetName() : TEXT("NoOwner");

	// Possessed PlayerId (있으면)
	int32 PlayerId = -1;
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (const APlayerState* PS = Pawn->GetPlayerState())
		{
			PlayerId = PS->GetPlayerId();
		}
	}

	return FString::Printf(TEXT("[%s][Owner=%s][LRole=%d RRole=%d][PID=%d]"),
		*NetModeStr, *OwnerName, (int32)LocalRole, (int32)RemoteRole, PlayerId);
}

// Sets default values for this component's properties
USkillComponent_SK_CO_RE_001::USkillComponent_SK_CO_RE_001()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USkillComponent_SK_CO_RE_001::BeginPlay()
{
	Super::BeginPlay();

	// 의존 컴포넌트 캐시 (필수)
	StatComponentCached = GetOwner() ? GetOwner()->FindComponentByClass<UStatComponent_SK_CO_RE_001>() : nullptr;

	BuildCacheFromDataTable();
}

void USkillComponent_SK_CO_RE_001::RequestActivateSkill(const FName SkillId, const FSkillIntent& Intent)
{
	if (!GetOwner()) return;

	// 클라면 요청만
	if (!GetOwner()->HasAuthority())
	{
		Server_TryActivateSkill(SkillId, Intent);
		return;
	}

	// 서버(리슨 포함)면 서버 로직 수행 후 전파까지 처리
	Server_TryActivateSkill(SkillId, Intent);
}

void USkillComponent_SK_CO_RE_001::Server_TryActivateSkill_Implementation(const FName SkillId,
	const FSkillIntent& Intent)
{
	FSkillResult Result;
	const bool bApproved = TryActivateSkill_InternalServer(SkillId, Intent, Result);

	if (bApproved)
	{
		// 성공: 모두에게
		Multicast_OnSkillResult(SkillId, Result, Intent);
	}
	else
	{
		// 실패: 요청자(Owner)에게만
		Client_OnSkillResult(SkillId, Result);
	}
}

void USkillComponent_SK_CO_RE_001::Multicast_OnSkillResult_Implementation(const FName SkillId,
	const FSkillResult& Result, const FSkillIntent& Intent)
{
	// 모든 클라(서버 포함)에서 연출 트리거 지점
	// Result.bSucceeded==true면 애니/이펙트 등 재생
	UE_LOG(LogSandboxSkill_CO, Log, TEXT("%s [Multicast] Skill=%s Success=%d Reason=%s"),
	*SK_NetTag(this), *SkillId.ToString(), Result.bSucceeded ? 1 : 0, SK_ToString(Result.FailReason));
}

void USkillComponent_SK_CO_RE_001::Client_OnSkillResult_Implementation(const FName SkillId, const FSkillResult& Result)
{
	// 요청자에게만 실패 이유 전달
	UE_LOG(LogSandboxSkill_CO, Log, TEXT("%s [ClientResult] Skill=%s Success=%d Reason=%s CooldownRemaining=%.2f, Stamina=%.1f/%.1f"),
		*SK_NetTag(this),
		*SkillId.ToString(),
		Result.bSucceeded ? 1 : 0,
		SK_ToString(Result.FailReason),
		Result.CooldownRemaining,
		Result.CurrentResource,
		Result.RequiredResource);
}

bool USkillComponent_SK_CO_RE_001::TryActivateSkill_InternalServer(const FName SkillId, const FSkillIntent& Intent,
	FSkillResult& OutResult)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	FSkillContext Context;
	FSkillResult Failure;
	if (!ResolveContext(SkillId, Context, Failure))
	{
		OutResult = Failure;
		return false;
	}
	
	const FSkillValidationResult ValidationResult = Validate(Context, Intent);
	if (!ValidationResult.bCanActivate)
	{
		OutResult.bSucceeded = false;
		OutResult.FailReason = ValidationResult.FailReason;
		OutResult.CooldownRemaining = ValidationResult.CooldownRemaining;
		OutResult.RequiredResource = ValidationResult.RequiredResource;
		OutResult.CurrentResource = ValidationResult.CurrentResource;
		return false;
	}
	
	Commit(Context, Intent);
	Execute(Context, Intent);

	OutResult.bSucceeded = true;
	OutResult.FailReason = ESkillFailReason::None;
	return true;
}

bool USkillComponent_SK_CO_RE_001::ResolveContext(const FName SkillId, FSkillContext& OutContext,
                                                  FSkillResult& OutFailure) const
{
	OutFailure = FSkillResult{};
	OutContext = FSkillContext{};
	
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		OutFailure.bSucceeded = false;
		OutFailure.FailReason = ESkillFailReason::MissingDependency;
		return false;
	}

	UStatComponent_SK_CO_RE_001* StatComponent = StatComponentCached.Get();
	if (!IsValid(StatComponent))
	{
		OutFailure.bSucceeded = false;
		OutFailure.FailReason = ESkillFailReason::MissingDependency;
		return false;
	}

	const FSkillDef* Def = nullptr;
	if(const FSkillDef* Found = SkillDefCache.Find(SkillId))
	{
		Def = Found;
	}	
	
	if(!Def)
	{
		OutFailure.bSucceeded = false;
		OutFailure.FailReason = ESkillFailReason::InvalidSkillId;
		return false;
	}

	const FSkillRuntimeState* Runtime = RuntimeStateMap.Find(SkillId);
	if(!Runtime)
	{
		// 캐시 누락 방어:
		// Resolve/Validate 단계에서 Add(쓰기)하면 설계 위반이라 여기선 실패 처리
		OutFailure.bSucceeded = false;
		OutFailure.FailReason = ESkillFailReason::InvalidSkillId; // 또는 MissingDependency.
		return false;
	}

	OutContext.OwnerActor = OwnerActor;
	OutContext.StatComponent = StatComponent;
	OutContext.SkillDef = Def;
	OutContext.RuntimeState = Runtime;
	return true;
}

FSkillValidationResult USkillComponent_SK_CO_RE_001::Validate(const FSkillContext& Context,
	const FSkillIntent& Intent) const
{
	FSkillValidationResult OutResult;

	// 1) 상태 체크 (읽기)
	if (Context.StatComponent && Context.StatComponent->IsInvalidSkillState())
	{
		OutResult.bCanActivate = false;
		OutResult.FailReason = ESkillFailReason::InvalidState;
		return OutResult;
	}
	
	// 2) 쿨타임 체크 (읽기)
	const float Now = GetServerNow();
	float Remaining = 0.f;
	if (Context.RuntimeState && Context.RuntimeState->IsOnCooldown(Now, Remaining))
	{
		OutResult.bCanActivate = false;
		OutResult.FailReason = ESkillFailReason::Cooldown;
		OutResult.CooldownRemaining = Remaining;
		return OutResult;
	}

	// 3) 자원 체크 (읽기)
	const float Cost = Context.SkillDef ? Context.SkillDef->ResourceCost : 0.f;
	const float Current = Context.StatComponent ? Context.StatComponent->GetStamina() : 0.f;

	OutResult.RequiredResource = Cost;
	OutResult.CurrentResource = Current;

	if (Current < Cost)
	{
		OutResult.bCanActivate = false;
		OutResult.FailReason = ESkillFailReason::NotEnoughResource;
		return OutResult;
	}

	// (옵션) Intent 기반 위치/타겟 검증도 여기서
	// 예: Origin/Forward가 이상하면 거절, 서버 기준으로 보정 등

	OutResult.bCanActivate = true;
	OutResult.FailReason = ESkillFailReason::None;
	return OutResult;
}

void USkillComponent_SK_CO_RE_001::Commit(const FSkillContext& Context, const FSkillIntent& Intent)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	// 자원 차감
	const float Cost = Context.SkillDef ? Context.SkillDef->ResourceCost : 0.f;
	const bool bConsumed = Context.StatComponent ? Context.StatComponent->ConsumeStamina(Cost) : false;
	if (!bConsumed)
	{
		// Validate와 Commit 사이 경합 방어(이론상)
		UE_LOG(LogSandboxSkill_CO, Warning, TEXT("%s [Commit] CousumeStamina failed unexpectedly."), *SK_NetTag(this));
	}

	// 쿨타임 시작 (서버 시간 기준)
	if(Context.SkillDef)
	{
		const float Now = GetServerNow();

		FSkillRuntimeState& Runtime = RuntimeStateMap.FindOrAdd(Context.SkillDef->SkillId);
		Runtime.CooldownEndTime = Now + Context.SkillDef->CooldownSeconds;
	}
}

void USkillComponent_SK_CO_RE_001::Execute(const FSkillContext& Context, const FSkillIntent& Intent)
{
	// 실제 게임플레이 영향 적용(데미지/투사체/상태이상 등)
	// v1에서는 로그만
	UE_LOG(LogSandboxSkill_CO, Log, TEXT("%s [Execute][Server] Skill=%s Origin=%s Forward=%s"),
		*SK_NetTag(this),
		*Context.SkillDef->SkillId.ToString(),
		*Intent.Origin.ToString(),
		*Intent.Forward.ToString());
}

float USkillComponent_SK_CO_RE_001::GetServerNow() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

void USkillComponent_SK_CO_RE_001::BuildCacheFromDataTable()
{
	SkillDefCache.Reset();

	if (!SkillDataTable) return;
	
	TArray<FSkillDef*> Rows;
	SkillDataTable->GetAllRows<FSkillDef>(TEXT("SkillDataTableLoad"), Rows);
	
	for(const FSkillDef* Row : Rows)
	{
		if (!Row) continue;;
		if (Row->SkillId.IsNone()) continue;
		
		SkillDefCache.Add(Row->SkillId, *Row);
		// 런타임 상태도 미리 슬롯 확보(선택)
		RuntimeStateMap.FindOrAdd(Row->SkillId);
	}
}