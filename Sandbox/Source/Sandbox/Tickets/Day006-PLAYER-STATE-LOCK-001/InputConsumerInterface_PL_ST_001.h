// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputConsumerInterface_PL_ST_001.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInputConsumerInterface_PL_ST_001 : public UInterface
{
	GENERATED_BODY()
};

class SANDBOX_API IInputConsumerInterface_PL_ST_001
{
	GENERATED_BODY()

public:
	// Axis2D: Move/Look 등 (이번 티켓은 Move만 사용)
	virtual void ConsumeAxis2D_Move(const FVector2D& Value) = 0;

	// Trigger: Attack/Interact/Skill 등 (이번 티켓은 Attack만 사용)
	virtual void ConsumeTrigger_Attack_Pressed() = 0;
	virtual void ConsumeTrigger_Attack_Released() = 0;
};
