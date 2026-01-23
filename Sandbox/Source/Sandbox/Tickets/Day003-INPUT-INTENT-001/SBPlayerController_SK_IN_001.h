// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBPlayerController_SK_IN_001.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputRebindComponent_SK_IN_001;
class UEnhancedInputComponent;

/**
 * 
 */
UCLASS()
class SANDBOX_API ASBPlayerController_SK_IN_001 : public APlayerController
{
	GENERATED_BODY()

public:
	ASBPlayerController_SK_IN_001();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 테스트용: Attack 눌렀을 때 로그만 찍음
	void OnAttackTriggered();
	
	// 콘솔 커맨드: UI 없이 리바인드
	UFUNCTION(Exec)
	void SM_RebindAttackKey(const FString& KeyName);

	UFUNCTION(Exec)
	void SM_DumpBindings();
	
protected:
	// 에디터에서 지정
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;
	
	UPROPERTY(VisibleAnywhere, Category = "Input")
	TObjectPtr<UInputRebindComponent_SK_IN_001> RebindComponent;
};
