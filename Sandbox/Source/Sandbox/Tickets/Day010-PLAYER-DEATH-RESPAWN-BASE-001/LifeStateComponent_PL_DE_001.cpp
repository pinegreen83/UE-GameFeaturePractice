// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/LifeStateComponent_PL_DE_001.h"
#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/StatComponent_PL_DE_001.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

ULifeStateComponent_PL_DE_001::ULifeStateComponent_PL_DE_001()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULifeStateComponent_PL_DE_001::BeginPlay()
{
	Super::BeginPlay();

	StatComp = GetOwner() ? GetOwner()->FindComponentByClass<UStatComponent_PL_DE_001>() : nullptr;
	
	// "죽음 조건"은 State가 결정하되, 트리거는 Stat 이벤트를 구독한다.
	if (StatComp)
	{
		StatComp->OnHealthDepleted.AddDynamic(this, &ULifeStateComponent_PL_DE_001::HandleHealthDepleted);
	}
}

void ULifeStateComponent_PL_DE_001::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StatComp)
	{
		StatComp->OnHealthDepleted.RemoveDynamic(this, &ULifeStateComponent_PL_DE_001::HandleHealthDepleted);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ULifeStateComponent_PL_DE_001::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULifeStateComponent_PL_DE_001, LifeState);
}

void ULifeStateComponent_PL_DE_001::HandleHealthDepleted()
{
	// 결정자는 항상 서버
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (LifeState == ELifeState_PL_DE_001::Dead)
	{
		return; // 중복 방지
	}

	UE_LOG(LogTemp, Log, TEXT("Is Dead"));
	
	SetLifeState_Server(ELifeState_PL_DE_001::Dead);

	// 데모: 3초 후 리스폰
	RequestRespawn_Server(20.f);
}

void ULifeStateComponent_PL_DE_001::SetLifeState_Server(ELifeState_PL_DE_001 NewState)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (LifeState == NewState)
	{
		return;
	}

	const ELifeState_PL_DE_001 Prev = LifeState;
	LifeState = NewState;
	
	UE_LOG(LogTemp, Log, TEXT("SetLifeState Now : %s, State : %s"), *GetOwner()->GetName(), *UEnum::GetValueAsString(LifeState));

	// 서버 자신도 즉시 반응해야 하므로 로컬 처리 호출
	HandleLifeStateChanged(Prev, NewState);
}

void ULifeStateComponent_PL_DE_001::OnRep_LifeState(ELifeState_PL_DE_001 Prev)
{
	UE_LOG(LogTemp, Log, TEXT("[OnRep_LifeState] Owner=%s Prev=%d New=%d"), *GetOwner()->GetName(), (int32)Prev, (int32)LifeState);	
	HandleLifeStateChanged(Prev, LifeState);
}

void ULifeStateComponent_PL_DE_001::HandleLifeStateChanged(ELifeState_PL_DE_001 Prev, ELifeState_PL_DE_001 NewState)
{
	OnLifeStateChanged.Broadcast(Prev, NewState);
}

void ULifeStateComponent_PL_DE_001::RequestRespawn_Server(float DelaySeconds)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (DelaySeconds <= 0.f)
	{
		SetLifeState_Server(ELifeState_PL_DE_001::Alive);
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	World->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		[this]()
		{
			// Alive 복귀
			SetLifeState_Server(ELifeState_PL_DE_001::Alive);

			// 예시: HP도 회복 (Stat은 숫자만)
			if (StatComp && GetOwner() && GetOwner()->HasAuthority())
			{
				const float FullHP = StatComp->GetMaxHP();
				// ApplyHPDelta_Server는 델타이므로 현재 HP를 채우는 방식
				const float Delta = FullHP - StatComp->GetHP();
				StatComp->ApplyHPDelta_Server(Delta);
			}
		},
		DelaySeconds,
		false
		);
}
