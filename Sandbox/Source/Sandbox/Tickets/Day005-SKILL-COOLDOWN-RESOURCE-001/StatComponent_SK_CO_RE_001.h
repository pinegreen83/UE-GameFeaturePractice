// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent_SK_CO_RE_001.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UStatComponent_SK_CO_RE_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	virtual float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	virtual bool ConsumeStamina(const float Amount)
	{
		if (Amount <= 0.f) return true;
		if (Stamina < Amount) return false;
		Stamina -= Amount;
		return true;
	}

	//  상태 체크가 별도 컴포넌트라면 여기 대신 그쪽으로 옮기면 됨.
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stat")
	virtual bool IsInvalidSkillState() const { return bInvalidSkillState; }

private:
	UPROPERTY(EditAnywhere, Category = "Sandbox|Stat")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Sandbox|Stat")
	bool bInvalidSkillState = false;
};
