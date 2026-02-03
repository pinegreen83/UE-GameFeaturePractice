// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/SBPlayerController_PL_DE_001.h"
#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/LifeStateComponent_PL_DE_001.h"
#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/StatComponent_PL_DE_001.h"
#include "GameFramework/Character.h"

void ASBPlayerController_PL_DE_001::TestDamage(float Damage)
{
	// 데미지는 양수로 입력 받는 걸 가정하고, 내부에서 -로 적용
	const float Clamped = FMath::Max(0.f, Damage);

	UE_LOG(LogTemp, Log, TEXT("[TestDamage] Input Damage=%.2f, Clamped=%.2f"), Damage, Clamped);
	
	// 서버면 바로 적용, 클라면 서버 RPC
	if (HasAuthority())
	{
		Server_ApplyDamage(Clamped);
	}
	else
	{
		Server_ApplyDamage(Clamped);
	}
}

void ASBPlayerController_PL_DE_001::PrintHP()
{
	if (UStatComponent_PL_DE_001* StatComp = GetStatComp())
	{
		UE_LOG(LogTemp, Log, TEXT("PrintHP : %.1f"), StatComp->GetHP());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PrintHP StatComp not found"));
	}
}

void ASBPlayerController_PL_DE_001::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BindLifeState(InPawn);
}

void ASBPlayerController_PL_DE_001::OnUnPossess()
{
	UnbindLifeState();
	Super::OnUnPossess();
}

void ASBPlayerController_PL_DE_001::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	UnbindLifeState();
	BindLifeState(InPawn);

	UE_LOG(LogTemp, Log, TEXT("[Set Pawn] PC=%s Local=%d Pawn=%s"), *GetNameSafe(this), IsLocalController(), *GetNameSafe(InPawn));
}

void ASBPlayerController_PL_DE_001::BindLifeState(APawn* InPawn)
{
	UnbindLifeState();

	if (!IsLocalController() || !InPawn)
	{
		return;
	}

	CachedLifeStateComp = InPawn->FindComponentByClass<ULifeStateComponent_PL_DE_001>();
	if (!CachedLifeStateComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("StateComp Failed"));
		return;
	}

	CachedLifeStateComp->OnLifeStateChanged.AddDynamic(this, &ASBPlayerController_PL_DE_001::HandleLifeStateChanged);
	
	// 현재 상태를 즉시 반영 (UI/입력 동기화)
	HandleLifeStateChanged(ELifeState_PL_DE_001::Alive, CachedLifeStateComp->GetLifeState());
}

void ASBPlayerController_PL_DE_001::UnbindLifeState()
{
	if (CachedLifeStateComp)
	{
		CachedLifeStateComp->OnLifeStateChanged.RemoveDynamic(this, &ASBPlayerController_PL_DE_001::HandleLifeStateChanged);
		CachedLifeStateComp = nullptr;
	}
}

void ASBPlayerController_PL_DE_001::Server_ApplyDamage_Implementation(float Damage)
{
	UE_LOG(LogTemp, Log, TEXT("[Server_ApplyDamage] Damage=%.2f"), Damage);

	if (UStatComponent_PL_DE_001* StatComp = GetStatComp())
	{
		// HP Delta이므로 데미지는 음수로
		StatComp->ApplyHPDelta_Server(-Damage);
	}
}

UStatComponent_PL_DE_001* ASBPlayerController_PL_DE_001::GetStatComp() const
{
	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		return Character->FindComponentByClass<UStatComponent_PL_DE_001>();
	}
	return nullptr;
}

void ASBPlayerController_PL_DE_001::HandleLifeStateChanged(ELifeState_PL_DE_001 Prev, ELifeState_PL_DE_001 NewState)
{
	// 입력 차단은 "소유자 로컬"에서만 의미가 있다.
	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HandleLifeStateChanged Character:%s"), *GetPawn()->GetName());

	if (NewState == ELifeState_PL_DE_001::Dead)
	{
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);

		// IMC  교체를 하고 싶으면 여기서 Enhanced Input Subsystem 통해서 처리하면 됨.
	}
	else if (NewState == ELifeState_PL_DE_001::Alive)
	{
		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);
	}
}
