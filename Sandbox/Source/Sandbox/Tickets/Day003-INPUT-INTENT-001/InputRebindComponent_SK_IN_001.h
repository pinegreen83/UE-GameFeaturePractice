// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputRebindComponent_SK_IN_001.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UInputRebindComponent_SK_IN_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	UInputRebindComponent_SK_IN_001();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// 이 IMC의 매핑을 런타임에서 교체한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebind")
	TObjectPtr<UInputMappingContext> TargetIMC;
	
	// 저장 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebind")
	FString SaveSlotName = TEXT("SandboxKeyBind");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebind")
	int32 UserIndex = 0;
	
	// UI에서 호출: 특정 InputAction의 키를 변경
	UFUNCTION(BlueprintCallable, Category = "Rebind")
	bool RebindActionKey(const UInputAction* Action, FKey NewKey, bool bSaveImmediately = true);
	
	// 시작 시 호출: 저장된 키세팅을 IMC에 적용
	UFUNCTION(BlueprintCallable, Category = "Rebind")
	bool LoadAndApply();

	// 현재 IMC 상태를 저장
	UFUNCTION(BlueprintCallable, Category = "Rebind")
	bool SaveCurrentBindings();

private:
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedSubsystem() const;
	bool ReplaceKeyInIMC(const UInputAction* Action, FKey NewKey);
};
