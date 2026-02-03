// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/StatComponent_PL_DE_001.h"
#include "Net/UnrealNetwork.h"

UStatComponent_PL_DE_001::UStatComponent_PL_DE_001()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UStatComponent_PL_DE_001::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStatComponent_PL_DE_001, HP);
}

void UStatComponent_PL_DE_001::ApplyHPDelta_Server(float Delta)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Apply HP Delta"));
	
	const float OldHP = HP;
	const float NewHP = FMath::Clamp(HP + Delta, 0, MaxHP);
	HP = NewHP;

	BroadcastHPEvents(OldHP, NewHP);
}

void UStatComponent_PL_DE_001::OnRep_HP(float OldHP)
{
	BroadcastHPEvents(OldHP, HP);
}

void UStatComponent_PL_DE_001::BroadcastHPEvents(float OldHP, float NewHP)
{
	const float Delta = NewHP - OldHP;

	// 항상 HP 변경은 알린다.
	OnHealthChanged.Broadcast(NewHP, Delta);

	// Depleted는 0 도달 시 단 한 번만
	if (!bDepletedBroadcasted && NewHP <= 0.f)
	{
		bDepletedBroadcasted = true;
		OnHealthDepleted.Broadcast();
	}

	// 리스폰 등으로 HP가 회복되면 다시 depleted 가능하도록 플래그 리셋
	if (bDepletedBroadcasted && NewHP > 0.f)
	{
		bDepletedBroadcasted = false;
	}
}