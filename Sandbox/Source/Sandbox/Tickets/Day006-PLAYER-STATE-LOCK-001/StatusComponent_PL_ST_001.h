// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent_PL_ST_001.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UStatusComponent_PL_ST_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatusComponent_PL_ST_001();

	UFUNCTION(BlueprintCallable)
	bool IsStunned() const { return bIsStunned; }

	// 서버에서만 상태 변경 (클라는 요청만)
	UFUNCTION(BlueprintCallable)
	void ToggleStun_ServerAuth();

	UFUNCTION(BlueprintCallable)
	void SetStun_ServerAuth(bool bNewStunned);
	
protected:
	virtual auto GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const -> void override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_IsStunned)
	bool bIsStunned = false;

	UFUNCTION()
	void OnRep_IsStunned();

	void ApplyStunEffect(); // 서버/클라 모두에서 시각/로컬 효과 적용 (이번 티켓은 이동 차단)
};
