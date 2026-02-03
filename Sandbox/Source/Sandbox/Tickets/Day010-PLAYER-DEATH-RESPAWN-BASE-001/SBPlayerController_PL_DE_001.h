// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatComponent_PL_DE_001.h"
#include "GameFramework/PlayerController.h"
#include "Tickets/Day010-PLAYER-DEATH-RESPAWN-BASE-001/LifeStateTypes_PL_DE_001.h"
#include "SBPlayerController_PL_DE_001.generated.h"

class ULifeStateComponent_PL_DE_001;

UCLASS()
class SANDBOX_API ASBPlayerController_PL_DE_001 : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void TestDamage(float Damage);

	UFUNCTION(Exec)
	void PrintHP();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetPawn(APawn* InPawn) override;

private:
	UPROPERTY()
	ULifeStateComponent_PL_DE_001* CachedLifeStateComp = nullptr;

	UFUNCTION()
	void HandleLifeStateChanged(ELifeState_PL_DE_001 Prev, ELifeState_PL_DE_001 NewState);

	void BindLifeState(APawn* InPawn);
	void UnbindLifeState();

	// 클라 콘솔에서 쳐도 서버에서 적용되도록 RPC
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(float Damage);
	
	// 내부 헬퍼
	UStatComponent_PL_DE_001* GetStatComp() const;
};
