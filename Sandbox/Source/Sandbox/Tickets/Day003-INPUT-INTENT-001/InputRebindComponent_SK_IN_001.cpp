// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day003-INPUT-INTENT-001/InputRebindComponent_SK_IN_001.h"
#include "Tickets/Day003-INPUT-INTENT-001/RebindSaveGame_SK_IN_001.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UInputRebindComponent_SK_IN_001::UInputRebindComponent_SK_IN_001()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInputRebindComponent_SK_IN_001::BeginPlay()
{
	Super::BeginPlay();

	LoadAndApply();
}

bool UInputRebindComponent_SK_IN_001::RebindActionKey(const UInputAction* Action, FKey NewKey, bool bSaveImmediately)
{
	if (!ReplaceKeyInIMC(Action, NewKey)) return false;
	return bSaveImmediately ? SaveCurrentBindings() : true;
}

bool UInputRebindComponent_SK_IN_001::LoadAndApply()
{
	if (!TargetIMC) return false;

	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		return true;
	}
	
	URebindSaveGame_SK_IN_001* SaveObject = Cast<URebindSaveGame_SK_IN_001>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	if (!SaveObject) return false;

	// 저장된 ActionName -> Key 를 IMC 매핑에 반영
	const TArray<FEnhancedActionKeyMapping>& Mappings = TargetIMC->GetMappings();

	bool bAnyApplied = false;
	for (FEnhancedActionKeyMapping Mapping : Mappings)
	{
		if (!Mapping.Action) continue;
		
		const FName ActionName = Mapping.Action->GetFName();
		if (const FKey* SavedKey = SaveObject->ActionToKey.Find(ActionName))
		{
			Mapping.Key = *SavedKey;
			bAnyApplied = true;
		}
	}

	if (bAnyApplied)
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedSubsystem())
		{
			Subsystem->RemoveMappingContext(TargetIMC);
			Subsystem->AddMappingContext(TargetIMC, 0);
		}
	}
	
	return true;
}

bool UInputRebindComponent_SK_IN_001::SaveCurrentBindings()
{
	if (!TargetIMC) return false;

	URebindSaveGame_SK_IN_001* SaveObject = Cast<URebindSaveGame_SK_IN_001>(UGameplayStatics::CreateSaveGameObject(URebindSaveGame_SK_IN_001::StaticClass()));
	if (!SaveObject) return false;

	for(const FEnhancedActionKeyMapping& Mapping : TargetIMC->GetMappings())
	{
		if(!Mapping.Action) continue;

		const FName ActionName = Mapping.Action->GetFName();
		SaveObject->ActionToKey.FindOrAdd(ActionName) = Mapping.Key;
	}

	return UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, UserIndex);
}

UEnhancedInputLocalPlayerSubsystem* UInputRebindComponent_SK_IN_001::GetEnhancedSubsystem() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController) return nullptr;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return nullptr;
	
	return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

bool UInputRebindComponent_SK_IN_001::ReplaceKeyInIMC(const UInputAction* Action, FKey NewKey)
{
	if (!TargetIMC || !Action) return false;

	// IMC 내부 매핑 배열을 순회하면서 해당 Action 매핑의 key를 교체
	const TArray<FEnhancedActionKeyMapping>& Mappings = TargetIMC->GetMappings();
	bool bReplaced = false;

	for (FEnhancedActionKeyMapping Mapping : Mappings)
	{
		if (Mapping.Action && Action && Mapping.Action->GetFName() == Action->GetFName())
		{
			UE_LOG(LogTemp, Display, TEXT("[SM] Compare: Mapping=%s (%p) vs Param=%s (%p), NewKey=%s"),
				*GetNameSafe(Mapping.Action), Mapping.Action.Get(),
				*GetNameSafe(Action), Action,
				*NewKey.ToString());
			
			Mapping.Key = NewKey;
			bReplaced = true;
			break;
		}
	}
	
	if (!bReplaced) return false;

	//	서브시스템에 "매핑이 바뀌었음"을 반영하려면 보통 IMC 재적용이 확실함.
	if(UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedSubsystem())
	{
		Subsystem->RemoveMappingContext(TargetIMC);
		Subsystem->AddMappingContext(TargetIMC, 0);
	}
	
	return true;
}
