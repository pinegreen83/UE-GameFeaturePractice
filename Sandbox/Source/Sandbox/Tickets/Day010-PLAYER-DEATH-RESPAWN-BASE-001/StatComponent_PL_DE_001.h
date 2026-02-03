// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent_PL_DE_001.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged_PL_DE_001, float, NewHp, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepleted_PL_DE_001);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UStatComponent_PL_DE_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatComponent_PL_DE_001();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	float GetHP() const { return HP; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	float GetMaxHP() const { return MaxHP; }

	// 서버 권한에서만 호출하는 것을 전제 (커밋 단계)
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	void ApplyHPDelta_Server(float Delta);

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Stat")
	FOnHealthChanged_PL_DE_001 OnHealthChanged;

	// "HP가 0이 되었다"는 사실 통지 (죽음 결정을 하지 않음)
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Stat")
	FOnHealthDepleted_PL_DE_001 OnHealthDepleted;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_HP, EditDefaultsOnly, Category = "Sandbox|Stat")
	float HP = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|Stat")
	float MaxHP = 100.f;

	// 중복 방지: HP가 0이 됐다는 이벤트를 한번만 쏘기 위한 플래그
	bool bDepletedBroadcasted = false;

	UFUNCTION()
	void OnRep_HP(float OldHP);

private:
	void BroadcastHPEvents(float OldHP, float NewHP);
};
