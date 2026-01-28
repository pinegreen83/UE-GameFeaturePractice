[Task ID]

INV-AUTH-BASE-001

[Title]

서버 Authority 기준 인벤토리의 “추가/제거/조회”를 일관된 API로 제공한다

[Background]

전투/스킬은 이벤트 중심이지만,
인벤토리는 “지속 상태”라서 실무에서 더 자주 터진다.
클라에서 임의로 아이템이 늘거나, 서버/클라 불일치가 생기면
재현도 어렵고 신뢰도도 무너진다.
그래서 인벤토리는 처음부터 서버 단일 소스 오브 트루스로 잡아야 한다.

[Requirements]
- UInventoryComponent(또는 유사 책임 단위)를 캐릭터에 부착
- 서버에서만 인벤토리 상태를 변경(Add/Remove)
- 클라는 “요청”만 하고, 서버가 확정한 결과를 수신한다
- 최소 기능
  - AddItem(ItemId, Count)
  - RemoveItem(ItemId, Count)
  - GetItemCount(ItemId)
- 실패/예외 케이스(최소 1개)
  - Remove 요청 시 수량 부족이면 실패 처리 + 인벤토리 변화 없음(로그로 확인)

[Constraints]
- UE 5.4
- Tick 금지: 이벤트/함수 호출 기반으로만 갱신
- 네트워크: 상태는 서버에서 Replicate(또는 RepNotify)로 동기화
- 확장성: 나중에 “보상 지급”, “퀘스트 조건 체크”가 이 컴포넌트 API만 호출하면 되게 설계
- GC/포인터: 데이터는 UPROPERTY 기반으로 안정적으로 유지

[Definition of Done]
- 리슨서버 2인 환경에서
  - 클라가 Add/Remove 요청 → 서버가 확정 → 양쪽 UI/로그에서 동일한 결과 확인
- 수량 부족 Remove 실패가 재현 가능하고, 상태가 절대 틀어지지 않음
- 인벤토리 변경 경로가 “서버 API 한 군데”로 고정됨