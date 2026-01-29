## Implementation Summary

이번 티켓에서는 MMORPG 스타일 슬롯 기반 인벤토리의 서버 Authority 구조를 구현했다.
인벤토리는 캐릭터에 부착된 UInventoryComponent가 관리하며, 모든 상태 변경(Add/Remove)은 서버에서만 수행된다.
클라이언트는 요청만 전달하고, 결과는 Replication을 통해 동기화된다.

디버그 입력은 PlayerController의 Exec 커맨드를 사용하여 요청 대상 플레이어가 명확히 확정되도록 구성했고,
로그를 통해 클라이언트 요청 → 서버 처리 → 복제 수신까지의 전체 흐름을 검증했다.

UI, 월드 픽업, 영속 저장(DB)은 의도적으로 범위에서 제외했다.

## Files
- InventoryComponent_IN_AU_001.h / .cpp
    - 인벤토리 슬롯 관리, 서버 Add/Remove, Replication
- InventorySlot.h
    - 슬롯 단위 아이템 상태 구조체
- ItemDefinition.h
    - 아이템 정의(DataTable Row)
- PlayerController_IN_AU_001.h / .cpp
    - 디버그용 Exec 커맨드(InvAdd / InvRemove / InvPrint)

## Core Logic
서버 Authority 단일 변경 경로
```
void UInventoryComponent_IN_AU_001::RequestAddItem(int32 ItemId, int32 Count)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		AddItem_Internal(ItemId, Count);
	}
	else
	{
		Server_AddItem(ItemId, Count);
	}
}
```

슬롯 기반 AddItem (MaxStack 고려)
- 기존 스택 채우기
- 빈 슬롯에 새 스택 생성
- 실제 추가된 수량만 이벤트로 브로드캐스트
```
const int32 AddedTotal = RequestCount - Remaining;
if (AddedTotal > 0)
{
	OnInventoryItemChanged.Broadcast(ItemId, AddedTotal, GetTotalItemCount(ItemId));
}
```

디버그 입력의 대상 확정
```
APawn* Pawn = GetPawn();
if (UInventoryComponent_IN_AU_001* Inv = Pawn->FindComponentByClass<UInventoryComponent_IN_AU_001>())
{
	Inv->RequestAddItem(ItemId, Count);
}
```

## Design Match Check
- 설계 일치
    - 서버 Authority 기반 인벤토리
    - 슬롯 단위 데이터 구조
    - 클라이언트 요청 / 서버 확정 / 복제 결과 수신
    - 이벤트 기반 변경 알림
- 구현 중 변경된 점
    - 디버그 커맨드를 Character가 아닌 PlayerController로 이동
        → 멀티 PIE 환경에서 요청 대상 혼선 방지

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- UI / 월드 픽업 / 저장 시스템과는 분리
- 추후 통합 시 별도 커밋 예정.