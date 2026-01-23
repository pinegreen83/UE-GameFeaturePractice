// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day003-INPUT-INTENT-001/SBPlayerController_SK_IN_001.h"
#include "Tickets/Day003-INPUT-INTENT-001/InputRebindComponent_SK_IN_001.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

ASBPlayerController_SK_IN_001::ASBPlayerController_SK_IN_001()
{
	RebindComponent = CreateDefaultSubobject<UInputRebindComponent_SK_IN_001>(TEXT("RebindComponent"));
}

void ASBPlayerController_SK_IN_001::BeginPlay()
{
	Super::BeginPlay();

	// 1) Enhanced Input Subsystem에 IMC 추가
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultIMC)
			{
				Subsystem->AddMappingContext(DefaultIMC, 0);
			}
		}
	}
	
	// 2) RebindComponent에 TargetIMC 세팅 + 저장된 키세팅 적용
	if (RebindComponent)
	{
		RebindComponent->TargetIMC = DefaultIMC;
		RebindComponent->LoadAndApply();
	}

	UE_LOG(LogTemp, Display, TEXT("[SM] Ready. Try console: SM_RebindAttackKey F"));
	UE_LOG(LogTemp, Display, TEXT("[SM] Or dump: SM_DumpBindings"));
}

void ASBPlayerController_SK_IN_001::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC || !IA_Attack)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SM] EnhancedInputComponent or IA_Attack missing."));
		return;
	}
	
	// IA_Attack이 트리거될 때 로그
	EIC->BindAction(IA_Attack, ETriggerEvent::Started, this, &ASBPlayerController_SK_IN_001::OnAttackTriggered);
}

void ASBPlayerController_SK_IN_001::OnAttackTriggered()
{
	UE_LOG(LogTemp, Display, TEXT("[SM] Attack Triggered!"));
}

void ASBPlayerController_SK_IN_001::SM_RebindAttackKey(const FString& KeyName)
{
	if (!RebindComponent || !IA_Attack)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SM] RebindComponent or IA_Attack missing."));
		return;
	}

	const FKey NewKey(*KeyName);
	if(!NewKey.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SM] Invalid Key: %s"), *KeyName);
		return;
	}

	const bool bOk = RebindComponent->RebindActionKey(IA_Attack, NewKey, true);
	UE_LOG(LogTemp, Display, TEXT("[SM] Rebind IA_Attack -> %s : %s"), *KeyName, bOk ? TEXT("OK") : TEXT("Fail"));
}

void ASBPlayerController_SK_IN_001::SM_DumpBindings()
{
	if (!DefaultIMC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SM] DefaultIMC missing."));
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("==== [SM] IMC Bindings Dump ===="));
	for(const FEnhancedActionKeyMapping& Mapping : DefaultIMC->GetMappings())
	{
		const FString ActionName = Mapping.Action ? Mapping.Action->GetName() : TEXT("None");
		UE_LOG(LogTemp, Display, TEXT("Action=%s Key=%s"), *ActionName, *Mapping.Key.ToString());
	}
}
