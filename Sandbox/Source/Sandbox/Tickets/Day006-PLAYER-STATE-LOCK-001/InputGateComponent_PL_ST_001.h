// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputGateComponent_PL_ST_001.generated.h"

UENUM()
enum class EInputRejectReason : uint8
{
	None,
	MissingPawn,
	MissingStatus,
	Stunned,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputRejected, FName, InputName, EInputRejectReason, Reason);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UInputGateComponent_PL_ST_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInputGateComponent_PL_ST_001();

	// Possess/리스폰 대비: PlayerController에서 Pawn 변경될 때 갱신
	void SetControlledPawn(APawn* NewPawn);

	// 타입별 입력 요청
	void RequestMoveAxis2D(const FVector2D& Value);
	void RequestAttackPressed();
	void RequestAttackReleased();

	UPROPERTY(BlueprintAssignable)
	FOnInputRejected OnInputRejected;

private:
	TWeakObjectPtr<APawn> ControlledPawn;

	EInputRejectReason CheckCommonPolicy() const;
};
