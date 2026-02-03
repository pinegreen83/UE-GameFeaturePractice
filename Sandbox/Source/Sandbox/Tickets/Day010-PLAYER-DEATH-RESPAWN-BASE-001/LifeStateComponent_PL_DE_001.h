// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LifeStateTypes_PL_DE_001.h"
#include "LifeStateComponent_PL_DE_001.generated.h"

class UStatComponent_PL_DE_001;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifeStateChanged_PL_DE_001, ELifeState_PL_DE_001, Prev, ELifeState_PL_DE_001, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API ULifeStateComponent_PL_DE_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	ULifeStateComponent_PL_DE_001();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|LifeState")
	ELifeState_PL_DE_001 GetLifeState() const { return LifeState; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|LifeState")
	bool bIsDead() const { return LifeState == ELifeState_PL_DE_001::Dead; }

	// 상태 전환은 여기 단일 진입점으로만
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LifeState")
	void SetLifeState_Server(ELifeState_PL_DE_001 NewState);

	// 리스폰(Alive 복귀)도 여기서 처리 가능
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LifeState")
	void RequestRespawn_Server(float DelaySeconds);
	
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|LifeState")
	FOnLifeStateChanged_PL_DE_001 OnLifeStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_LifeState)
	ELifeState_PL_DE_001 LifeState = ELifeState_PL_DE_001::Alive;

	UFUNCTION()
	void OnRep_LifeState(ELifeState_PL_DE_001 Prev);

	void HandleLifeStateChanged(ELifeState_PL_DE_001 Prev, ELifeState_PL_DE_001 NewState);

	UFUNCTION()
	void HandleHealthDepleted(); // Stat 이벤트 구독

	UPROPERTY()
	UStatComponent_PL_DE_001* StatComp = nullptr;

	FTimerHandle RespawnTimerHandle;
};
