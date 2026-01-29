// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tickets/Day007-INV-AUTH-BASE-001/InventorySlot_IN_AU_001.h"
#include "InventoryComponent_IN_AU_001.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInventoryItemChanged,
	int32, ItemId,
	int32, DeltaCount,
	int32, NewTotalCount
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UInventoryComponent_IN_AU_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent_IN_AU_001();

	/* === Client Request === */
	void RequestAddItem(int32 ItemId, int32 Count);
	void RequestRemoveItem(int32 ItemId, int32 Count);
	
	/* === Read Only === */
	int32 GetItemCount(int32 ItemId) const;

	/* === Event === */
	UPROPERTY(BlueprintAssignable)
	FOnInventoryItemChanged OnInventoryItemChanged;

protected:
	virtual void BeginPlay() override;

	/* === Server RPC === */
	UFUNCTION(Server, Reliable)
	void Server_AddItem(int32 ItemId, int32 Count);

	UFUNCTION(Server, Reliable)
	void Server_RemoveItem(int32 ItemId, int32 Count);
	
	/* === Internal Logic (Server Only) === */
	bool AddItem_Internal(int32 ItemId, int32 Count);
	bool RemoveItem_Internal(int32 ItemId, int32 Count);

	/* === Replication === */
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<FInventorySlot> Slots;

	UFUNCTION()
	void OnRep_Slots();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	int32 GetTotalItemCount(int32 ItemId) const;

	int32 GetMaxStackCount(int32 ItemId) const;
};
