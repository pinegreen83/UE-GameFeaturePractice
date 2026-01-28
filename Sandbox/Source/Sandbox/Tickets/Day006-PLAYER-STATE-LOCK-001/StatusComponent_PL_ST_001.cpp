// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day006-PLAYER-STATE-LOCK-001/StatusComponent_PL_ST_001.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UStatusComponent_PL_ST_001::UStatusComponent_PL_ST_001()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatusComponent_PL_ST_001::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStatusComponent_PL_ST_001, bIsStunned);
}

void UStatusComponent_PL_ST_001::ToggleStun_ServerAuth()
{
	SetStun_ServerAuth(!bIsStunned);
}

void UStatusComponent_PL_ST_001::SetStun_ServerAuth(bool bNewStunned)
{
	// 서버만 변경
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bIsStunned == bNewStunned)
	{
		return;
	}

	bIsStunned = bNewStunned;
	ApplyStunEffect();
	// RepNotify는 클라에서 OnRep으로 처리됨.
}

void UStatusComponent_PL_ST_001::OnRep_IsStunned()
{
	ApplyStunEffect();
}

void UStatusComponent_PL_ST_001::ApplyStunEffect()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent) return;

	if (bIsStunned)
	{
		MovementComponent->DisableMovement(); // 서버에서 확정 -> 클라도 일관성 유지
	}
	else
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
}