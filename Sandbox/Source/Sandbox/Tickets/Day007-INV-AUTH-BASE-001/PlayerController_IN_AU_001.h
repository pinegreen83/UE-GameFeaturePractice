// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_IN_AU_001.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API APlayerController_IN_AU_001 : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void InvAdd(int32 ItemId, int32 Count);
	
	UFUNCTION(Exec)
	void InvRemove(int32 ItemId, int32 Count);

	UFUNCTION(Exec)
	void InvPrint();
	
};
