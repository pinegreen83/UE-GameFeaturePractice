// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day001-SKILL-CORE-001/SkillComponent_SKILL_CORE_001.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

static FCooldownEntry_SKILL_CORE_001* FindCooldownEntry(TArray<FCooldownEntry_SKILL_CORE_001>& Arr, FName SkillId)
{
	for (FCooldownEntry_SKILL_CORE_001& Entry : Arr)
	{
		if(Entry.SkillId == SkillId)
		{
			return &Entry;
		}
	}
	return nullptr;
}

USkillComponent_SKILL_CORE_001::USkillComponent_SKILL_CORE_001()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USkillComponent_SKILL_CORE_001::BeginPlay()
{
	Super::BeginPlay();
}

void USkillComponent_SKILL_CORE_001::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USkillComponent_SKILL_CORE_001, CooldownEntries_Replicated);
}

bool USkillComponent_SKILL_CORE_001::RequestActivateSkill(FName SkillId, const FSkillIntentData_SKILL_CORE_001& Intent)
{
	const FSkillDataRow_SKILL_CORE_001* Row = FindSkillRow(SkillId);
	if (!Row || Row->Policy != ESkillActivationPolicy::Active)
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_CORE_001] Invalid SkillId or not Active : %s"), *SkillId.ToString());
		return false;
	}

	FSkillExecutionContext_SKILL_CORE_001 CandidateCtx;
	if(!BuildCandidateContext(CandidateCtx, Intent))
	{
		return false;
	}
	
	LogSkillEvent(TEXT("RequestActivateSkill"), SkillId, &CandidateCtx);
	
	// 클라 예측 연출(최소) : 서버 승인 전이라도 "눌림" 피드백은 줌.
	// (거절 시 롤백은 이 티켓 범위에서 최소화: 로그만)
	if (!CandidateCtx.bHasAuthority)
	{
		FSkillCueData_SKILL_CORE_001 Cue;
		Cue.SkillId = SkillId;
		Cue.InstigatorActor = CandidateCtx.InstigatorActor;
		Cue.Origin = CandidateCtx.Origin;
		Cue.AimDir = CandidateCtx.AimDir;
		PlayCue_Local(Cue, *Row);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ServerTryActivateSkill(SkillId, Intent); // 권한 있으면 로컬 호출로도 돌아감.
	}
	else
	{
		ServerTryActivateSkill(SkillId, Intent);
	}

	return true;
}

void USkillComponent_SKILL_CORE_001::ServerTryActivateSkill_Implementation(FName SkillId, FSkillIntentData_SKILL_CORE_001 Intent)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	const FSkillDataRow_SKILL_CORE_001* Row = FindSkillRow(SkillId);
	if (!Row || Row->Policy != ESkillActivationPolicy::Active)
	{
		LogSkillEvent(TEXT("ServerReject_InvalidSkill"), SkillId);
		return;
	}

	FSkillExecutionContext_SKILL_CORE_001 CandidateCtx;
	if (!BuildCandidateContext(CandidateCtx, Intent))
	{
		LogSkillEvent(TEXT("ServerReject_BuildCtxFail"), SkillId);
		return;
	}

	// 서버가 최종 컨텍스트 확정(필요 시 여기서 재-trace/타겟 재검증 가능)
	FSkillExecutionContext_SKILL_CORE_001 AuthCtx = CandidateCtx;
	AuthCtx.bHasAuthority = true;

	if (!ValidateAndCommit_Server(SkillId, AuthCtx, *Row))
	{
		LogSkillEvent(TEXT("ServerReject_ValidateFail"), SkillId, &AuthCtx);
		return;
	}

	ExecuteSkill_Server(SkillId, AuthCtx, *Row);
}

float USkillComponent_SKILL_CORE_001::GetCooldownRemaining(FName SkillId) const
{
	const float* EndTime = CooldownEndTimes_Cache.Find(SkillId);
	if (!EndTime) return 0.f;
	
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return FMath::Max(0.f, *EndTime - Now);
}

bool USkillComponent_SKILL_CORE_001::IsSkillOnCooldown(FName SkillId) const
{
	return GetCooldownRemaining(SkillId) > 0.f;
}

void USkillComponent_SKILL_CORE_001::OnRep_CooldownEntries()
{
	CooldownEndTimes_Cache.Reset();

	for (const FCooldownEntry_SKILL_CORE_001& Entry : CooldownEntries_Replicated)
	{
		if (Entry.SkillId != NAME_None)
		{
			CooldownEndTimes_Cache.Add(Entry.SkillId, Entry.EndTime);
		}
	}
}

bool USkillComponent_SKILL_CORE_001::BuildCandidateContext(FSkillExecutionContext_SKILL_CORE_001& OutCtx,
                                                           const FSkillIntentData_SKILL_CORE_001& Intent) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	
	OutCtx.OwnerActor = OwnerActor;
	OutCtx.InstigatorActor = OwnerActor; // 기본은 Owner = Instigator. 소환수/대리시전은 여기를 바꿔야 됨.
	OutCtx.TargetActor = Intent.SuggestedTarget;
	OutCtx.Origin = Intent.Origin;
	OutCtx.AimDir = Intent.AimDir;
	OutCtx.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	OutCtx.bHasAuthority = OwnerActor->HasAuthority();

	return true;
}

bool USkillComponent_SKILL_CORE_001::ValidateAndCommit_Server(FName SkillId,
	const FSkillExecutionContext_SKILL_CORE_001& Ctx, const FSkillDataRow_SKILL_CORE_001& Row)
{
	// 1) 쿨다운 체크(서버 권위) - 캐시 기준
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float* EndTime = CooldownEndTimes_Cache.Find(SkillId);
	if(EndTime && *EndTime > Now)
	{
		return false;
	}
	
	// 2) 자원 체크/차감 TODO (서버 전용)
	
	// 3) 쿨다운 확정: EndTime 계산
	if (Row.Active.Cooldown > 0.f)
	{
		const float NewEnd = Now + Row.Active.Cooldown;
		
		// 캐시 업데이트
		CooldownEndTimes_Cache.Add(SkillId, NewEnd);

		// 복제 배열 업데이트
		if(FCooldownEntry_SKILL_CORE_001* Existing = FindCooldownEntry(CooldownEntries_Replicated, SkillId))
		{
			Existing->EndTime = NewEnd;
		}
		else
		{
			FCooldownEntry_SKILL_CORE_001 NewEntry;
			NewEntry.SkillId = SkillId;
			NewEntry.EndTime = NewEnd;
			CooldownEntries_Replicated.Add(NewEntry);
		}
	}
	
	return true;
}

void USkillComponent_SKILL_CORE_001::ExecuteSkill_Server(FName SkillId,
	const FSkillExecutionContext_SKILL_CORE_001& AuthCtx, const FSkillDataRow_SKILL_CORE_001& Row)
{
	check(GetOwner() && GetOwner()->HasAuthority());

	LogSkillEvent(TEXT("ServerExecuteSkill"), SkillId, &AuthCtx);
	UWorld* World = GetWorld();
	if (!World)	return;

	const FVector Start = AuthCtx.Origin;
	const FVector End = Start + (FVector(AuthCtx.AimDir) * Row.Active.Range);
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SKILL_CORE_001_HitScan), false);
	Params.AddIgnoredActor(AuthCtx.OwnerActor);

	UE_LOG(LogTemp, Log, TEXT("Skill Test IsTracing"));
	
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_CORE_001] Hit Test"));
	}
	
	if (AActor* HitActor = bHit ? Hit.GetActor(): nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[SKILL_CORE_001] ApplyDamage on SERVER. Victim = %s Damage = %.2f"), *HitActor->GetName(), Row.Active.Damage);
		
		// 서버에서만 데미지 적용
		UGameplayStatics::ApplyDamage(
			HitActor,
			Row.Active.Damage,
			nullptr,
			AuthCtx.InstigatorActor,
			UDamageType::StaticClass()
		);
	}

	// ---- Cue 전파(연출) ----
	FSkillCueData_SKILL_CORE_001 Cue;
	Cue.SkillId = SkillId;
	Cue.InstigatorActor = AuthCtx.InstigatorActor;
	Cue.Origin = AuthCtx.Origin;
	Cue.AimDir = AuthCtx.AimDir;
	MulticastPlaySkillCue(Cue);
}

void USkillComponent_SKILL_CORE_001::MulticastPlaySkillCue_Implementation(FSkillCueData_SKILL_CORE_001 Cue)
{
	const FSkillDataRow_SKILL_CORE_001* Row = FindSkillRow(Cue.SkillId);
	if (!Row) return;
	
	PlayCue_Local(Cue, *Row);
}

void USkillComponent_SKILL_CORE_001::PlayCue_Local(const FSkillCueData_SKILL_CORE_001& Cue,
	const FSkillDataRow_SKILL_CORE_001& Row) const
{
	// 로컬 연출 : Montage / SFX / VFX (SoftObject는 필요 시 로드)
	AActor* Inst = Cue.InstigatorActor;
	if (!Inst) return;

	// SFX
	if (Row.Active.SFX.IsValid() || !Row.Active.SFX.ToSoftObjectPath().IsNull())
	{
		if(USoundBase* Sound = Row.Active.SFX.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Inst->GetActorLocation());
		}
	}

	// VFX
	if (Row.Active.VFX.IsValid() || !Row.Active.VFX.ToSoftObjectPath().IsNull())
	{
		if (UParticleSystem* Particle = Row.Active.VFX.LoadSynchronous())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, Inst->GetActorTransform());
		}
	}

	// Montage (캐릭터만 대상)
	if (Row.Active.Montage.IsValid() || !Row.Active.Montage.ToSoftObjectPath().IsNull())
	{
		if (UAnimMontage* Montage = Row.Active.Montage.LoadSynchronous())
		{
			APawn* Pawn = Cast<APawn>(Inst);
			if (ACharacter* Character = Pawn ? Cast<ACharacter>(Pawn) : Cast<ACharacter>(Inst))
			{
				if (UAnimInstance* Anim = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
				{
					Anim->Montage_Play(Montage);
				}
			}
		}
	}
}

const FSkillDataRow_SKILL_CORE_001* USkillComponent_SKILL_CORE_001::FindSkillRow(FName SkillId) const
{
	if (!SkillDataTable) return nullptr;
	return SkillDataTable->FindRow<FSkillDataRow_SKILL_CORE_001>(SkillId, TEXT("SkillDataLookup"));
}

void USkillComponent_SKILL_CORE_001::LogSkillEvent(const TCHAR* Event, FName SkillId,
	const FSkillExecutionContext_SKILL_CORE_001* Ctx) const
{
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	
	const ENetMode NetMode = World ? World->GetNetMode() : ENetMode::NM_Standalone;
	const FString NetModeStr =
		(NetMode == NM_Standalone) ? TEXT("Standalone") :
		(NetMode == NM_ListenServer) ? TEXT("ListenServer") :
		(NetMode == NM_DedicatedServer) ? TEXT("DedicatedServer") : TEXT("Client");

	const FString OwnerName = OwnerActor ? OwnerActor->GetName() : TEXT("None");
	FString CtxStr;
	if (Ctx)
	{
		const FString Inst = Ctx->InstigatorActor ? Ctx->InstigatorActor->GetName() : TEXT("None");
		const FString Tgt = Ctx->TargetActor ? Ctx->TargetActor->GetName() : TEXT("None");
		CtxStr = FString::Printf(TEXT(" Instigator = %s Target = %s Auth = %d"), *Inst, *Tgt, Ctx->bHasAuthority ? 1 : 0);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[SKILL_CORE_OO1][%s] %s SkillId = %s Owner = %s%s"),
		*NetModeStr, Event, *SkillId.ToString(), *OwnerName, *CtxStr);
}
