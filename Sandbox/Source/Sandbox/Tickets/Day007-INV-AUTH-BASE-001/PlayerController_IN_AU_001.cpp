// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day007-INV-AUTH-BASE-001/PlayerController_IN_AU_001.h"
#include "GameFramework/Pawn.h"
#include "Tickets/Day007-INV-AUTH-BASE-001/InventoryComponent_IN_AU_001.h"

void APlayerController_IN_AU_001::InvAdd(int32 ItemId, int32 Count)
{
	UE_LOG(LogTemp, Log, TEXT("InvAdd Command"));
	
	APawn* Pawn = GetPawn();

	if (!Pawn)
	{
		UE_LOG(LogTemp, Log, TEXT("Cant Find Pawn"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Pawn=%s Class=%s"),
		*GetNameSafe(Pawn),
		*GetNameSafe(Pawn->GetClass()));
	
	if (UInventoryComponent_IN_AU_001* ItemInventory = Pawn ? Pawn->FindComponentByClass<UInventoryComponent_IN_AU_001>() : nullptr)
	{
		ItemInventory->RequestAddItem(ItemId, Count);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cant Find Inventory"));
	}
}

void APlayerController_IN_AU_001::InvRemove(int32 ItemId, int32 Count)
{
	UE_LOG(LogTemp, Log, TEXT("InvRemove Command"));
	
	APawn* Pawn = GetPawn();

	if (!Pawn)
	{
		UE_LOG(LogTemp, Log, TEXT("Cant Find Pawn"));
		return;
	}
	
	if (UInventoryComponent_IN_AU_001* ItemInventory = Pawn ? Pawn->FindComponentByClass<UInventoryComponent_IN_AU_001>() : nullptr)
	{
		ItemInventory->RequestRemoveItem(ItemId, Count);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Cant Find Inventory"));
	}
}

void APlayerController_IN_AU_001::InvPrint()
{
	APawn* Pawn = GetPawn();
	
}
