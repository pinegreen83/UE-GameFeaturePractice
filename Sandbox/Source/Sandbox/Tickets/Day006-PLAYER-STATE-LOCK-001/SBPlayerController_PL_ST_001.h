// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBPlayerController_PL_ST_001.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputGateComponent_PL_ST_001;
struct FInputActionValue;

UCLASS()
class SANDBOX_API ASBPlayerController_PL_ST_001 : public APlayerController
{
	GENERATED_BODY()

public:
	ASBPlayerController_PL_ST_001();

protected:
	virtual void BeginPlay()override;
	virtual void OnPossess(APawn* inPawn)override;
	virtual void SetupInputComponent()override;

	// Exec: 어디서 치든 "자기 Pawn만" 토글
	UFUNCTION(EXEC)
	void ToggleStunSelf();

	UFUNCTION(Server, Reliable)
	void Server_ToggleStunSelf();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInputGateComponent_PL_ST_001> InputGateComponent;

	// Enhanced Input Assets
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InGameIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;

	// Callbacks (얇은 어댑터)
	void OnMoveTriggered(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);
	
	void OnAttackStarted(const FInputActionValue& Value);
	void OnAttackCompleted(const FInputActionValue& Value);
	
	void ApplyIMC();
	void RefreshGatePawn();
};