#pragma once

#include "CoreMinimal.h"
#include "InventorySlot_IN_AU_001.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemId = INDEX_NONE;

	UPROPERTY()
	int32 Count = 0;

	bool IsEmpty() const
	{
		return ItemId == INDEX_NONE || Count <= 0;
	}

	void Clear()
	{
		ItemId = INDEX_NONE;
		Count = 0;
	}
};