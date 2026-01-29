// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day007-INV-AUTH-BASE-001/InventoryComponent_IN_AU_001.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogInventory, Log, All);

UInventoryComponent_IN_AU_001::UInventoryComponent_IN_AU_001()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent_IN_AU_001::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Slots.SetNum(20); // 고정 슬롯 수 (오늘 범위)
	}
}

/* ================= Client =============== */

void UInventoryComponent_IN_AU_001::RequestAddItem(int32 ItemId, int32 Count)
{
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	APawn* Pawn = Cast<APawn>(Owner);
    APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
    
    UE_LOG(LogInventory, Log, TEXT("[RequestAddItem][IsServer=%d][Owner=%s][PlayerId=%d][LocalRole=%d] ItemTotalCount = %d")
    	, Owner && Owner->HasAuthority()
    	, *GetNameSafe(Owner)
    	, PS ? PS->GetPlayerId() : -1
    	, Owner ? (int32)Owner->GetLocalRole() : -1
    	, GetTotalItemCount(ItemId));

	if (bIsServer)
	{
		const bool bAllAdded = AddItem_Internal(ItemId, Count);
		UE_LOG(LogInventory, Log, TEXT("[RequestAddItem][ServerDirect] bAllAdded=%d"), bAllAdded);
	}
	else
	{
		Server_AddItem(ItemId, Count);
	}

	UE_LOG(LogInventory, Log, TEXT("[RequestAddItemFinish][IsServer=%d][Owner=%s][PlayerId=%d][LocalRole=%d] ItemTotalCount = %d")
		, Owner && Owner->HasAuthority()
		, *GetNameSafe(Owner)
		, PS ? PS->GetPlayerId() : -1
		, Owner ? (int32)Owner->GetLocalRole() : -1
		, GetTotalItemCount(ItemId));
}

void UInventoryComponent_IN_AU_001::RequestRemoveItem(int32 ItemId, int32 Count)
{
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	APawn* Pawn = Cast<APawn>(Owner);
	APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;

	UE_LOG(LogInventory, Log, TEXT("[RequestRemoveItem][IsServer=%d][Owner=%s][PlayerId=%d][LocalRole=%d]")
		, Owner && Owner->HasAuthority()
		, *GetNameSafe(Owner)
		, PS ? PS->GetPlayerId() : -1
		, Owner ? (int32)Owner->GetLocalRole() : -1);
	
	if (bIsServer)
	{
		const bool bOk = RemoveItem_Internal(ItemId, Count);
		UE_LOG(LogInventory, Log, TEXT("[RequestRemoveItem][ServerDirect] bOk=%d"), bOk);
	}
	else
	{
		Server_RemoveItem(ItemId, Count);
	}
}

/* ================= Server RPC ================== */

void UInventoryComponent_IN_AU_001::Server_AddItem_Implementation(int32 ItemId, int32 Count)
{
	UE_LOG(LogInventory, Log, TEXT("[ServerAdd][Owner=%s] ItemId=%d Count=%d"), *GetNameSafe(GetOwner()), ItemId, Count);
	
	const bool bAllAdded = AddItem_Internal(ItemId, Count);

	UE_LOG(LogInventory, Log, TEXT("[ServerAddResult][Owner=%s] bAllAdded=%d Total=%d"), *GetNameSafe(GetOwner()), bAllAdded, GetTotalItemCount(ItemId));
}

void UInventoryComponent_IN_AU_001::Server_RemoveItem_Implementation(int32 ItemId, int32 Count)
{
	UE_LOG(LogInventory, Log, TEXT("[ServerRemove][Owner=%s] ItemId=%d Count=%d"), *GetNameSafe(GetOwner()), ItemId, Count);
	
	const bool bAllRemoved = RemoveItem_Internal(ItemId, Count);
	
	UE_LOG(LogInventory, Log, TEXT("[ServerRemoveResult][Owner=%s] bAllRemoved=%d Total=%d"), *GetNameSafe(GetOwner()), bAllRemoved, GetTotalItemCount(ItemId));
}

/* ================= Internal Logic ================= */

bool UInventoryComponent_IN_AU_001::AddItem_Internal(int32 ItemId, int32 Count)
{
	const AActor* Owner = GetOwner();
	const APawn* Pawn = Cast<APawn>(Owner);
	APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;

	UE_LOG(LogInventory, Log, TEXT("[AddItem_Internal][IsServer=%d][Owner=%s][PlayerId=%d][LocalRole=%d]")
		, Owner && Owner->HasAuthority()
		, *GetNameSafe(Owner)
		, PS ? PS->GetPlayerId() : -1
		, Owner ? (int32)Owner->GetLocalRole() : -1);
	
	if (!Owner || !Owner->HasAuthority() || ItemId == INDEX_NONE || Count <= 0)
	{
		return false;
	}

	int32 Remaining = Count;
	const int32 MaxStack = FMath::Max(1, GetMaxStackCount(ItemId));
	
	// 1. 기존 스택 합치기
	for(FInventorySlot& Slot : Slots)
	{
		if (Remaining <= 0) break;
		
		if (Slot.ItemId == ItemId && Slot.Count > 0 && Slot.Count < MaxStack)
		{
			const int32 Space = MaxStack - Slot.Count;
			const int32 AddAmount = FMath::Min(Space, Remaining);
			
			Slot.Count += AddAmount;
			Remaining -= AddAmount;
		}
	}
	
	// 2. 빈 슬롯에 새 스택 생성
	for (FInventorySlot& Slot : Slots)
	{
		if (Remaining <= 0) break;
		
		if(Slot.IsEmpty())
		{
			const int32 AddAmount = FMath::Min(MaxStack, Remaining);

			Slot.ItemId = ItemId;
			Slot.Count = AddAmount;
			Remaining -= AddAmount;
		}
	}

	const int32 AddTotal = Count - Remaining;
	if (AddTotal > 0)
	{
		OnInventoryItemChanged.Broadcast(ItemId, AddTotal, GetTotalItemCount(ItemId));
	}

	// Remaining ? 0 이면 인벤 공간 부족으로 "부분 성공"
	return (Remaining == 0);
}

bool UInventoryComponent_IN_AU_001::RemoveItem_Internal(int32 ItemId, int32 Count)
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || ItemId == INDEX_NONE || Count <= 0)
	{
		return false;
	}
	
	int32 Remaining = Count;
	
	for (FInventorySlot& Slot : Slots)
	{
		if (Remaining <= 0) break;
		
		if (Slot.ItemId == ItemId && Slot.Count > 0)
		{
			const int32 RemoveAmount = FMath::Min(Slot.Count, Remaining);
			Slot.Count -= RemoveAmount;
			Remaining -= RemoveAmount;
			
			if(Slot.Count <= 0)
			{
				Slot.Clear();
			}
		}
	}

	if(Remaining == 0)
	{
		OnInventoryItemChanged.Broadcast(ItemId, -Count, GetTotalItemCount(ItemId));
		return true;
	}

	// 수량 부족: 상태 롤백은 이번 티켓 범위 밖 (다음 티켓에서 트랜잭션 처리 가능)
	return false;
}

/* ==================== Replication ========================*/

void UInventoryComponent_IN_AU_001::OnRep_Slots()
{
	UE_LOG(LogInventory, Log, TEXT("[OnRep_Slots][Owner=%s] Slots=%d"), *GetNameSafe(GetOwner()), Slots.Num());
	// UI / 디버그용
}

void UInventoryComponent_IN_AU_001::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent_IN_AU_001, Slots);
}

/* ===================== Utility ===================== */

int32 UInventoryComponent_IN_AU_001::GetItemCount(int32 ItemId) const
{
	return GetTotalItemCount(ItemId);
}

int32 UInventoryComponent_IN_AU_001::GetTotalItemCount(int32 ItemId) const
{
	int32 Total = 0;
	for(const FInventorySlot& Slot : Slots)
	{
		if (Slot.ItemId == ItemId)
		{
			Total += Slot.Count;
		}
	}
	
	return Total;
}

int32 UInventoryComponent_IN_AU_001::GetMaxStackCount(int32 ItemId) const
{
	// TODO: ItemDef DataTable에서 ItemId(RowName)로 찾아 MaxStackCount 반환
	// 오늘 티켓 범위에서는 하드코딩으로 시작 (추후 DataTable 바인딩 티켓에서 교체)
	return 99;
}
