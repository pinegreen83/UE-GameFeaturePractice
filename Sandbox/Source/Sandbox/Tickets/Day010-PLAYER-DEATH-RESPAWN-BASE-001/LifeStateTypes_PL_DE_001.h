#pragma once

#include "CoreMinimal.h"
#include "LifeStateTypes_PL_DE_001.generated.h"

UENUM(BlueprintType)
enum class ELifeState_PL_DE_001 : uint8
{
	Alive	UMETA(DisplayName = "Alive"),
	Dead	UMETA(DisplayName = "Dead"),
};