#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemType_IN_AU_001.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equip,
	Consumable,
	Misc
};

USTRUCT(BlueprintType)
struct FItemDefRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStackCount = 1;
};