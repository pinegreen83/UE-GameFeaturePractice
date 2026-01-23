// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RebindSaveGame_SK_IN_001.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API URebindSaveGame_SK_IN_001 : public USaveGame
{
	GENERATED_BODY()

public:
	// 예: "IA_Attack" -> EKeys::LeftMouseButton
	UPROPERTY()
	TMap<FName, FKey> ActionToKey;
	
};
