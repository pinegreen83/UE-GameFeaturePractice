[Task ID]

Day011-NET-VISIBILITY-RELEVANCE-001

[Title]

멀티플레이 환경에서 액터의 네트워크 가시성 및 Relevancy를 통제한다

[Background]

현재까지 구현된 월드 상호작용, 사망/리스폰, 스킬 사용은
“동작은 되지만, 모두에게 항상 보이는 상태”를 전제로 한다.

실무에서는
- 왜 어떤 액터는 보이고
- 왜 어떤 이벤트는 안 보이며
- 왜 특정 클라이언트에서만 상태가 어긋나는지
    
    의 80%가 Net Relevancy / Dormancy / Owner 기준에서 터진다.

이 티켓은 Replication 자체가 아니라 ‘누구에게 보이느냐’를 통제하는 단계다.

[Requirements]
- 서버 권한 액터 기준으로 다음을 구분한다
    - 항상 모든 클라이언트에게 보여야 하는 액터
    - 거리/조건에 따라 보이지 않아야 하는 액터
    - Owner에게만 의미 있는 액터
- bOnlyRelevantToOwner, NetCullDistanceSquared 중 최소 1개 적용
- Listen Server / Client 각각에서 로그로 Relevancy 차이를 확인 가능해야 한다
- 실패 케이스:
    - 클라이언트에서 보이면 안 되는 액터가 보이는 경우 로그로 탐지

[Constraints]
- UE 5.4 기준
- Tick 기반 거리 계산 금지 (엔진 Relevancy 사용)
- Replication 조건은 명시적으로 설정
- 추후 AI / 월드 이벤트에도 재사용 가능한 구조여야 함

[Definition of Done]
- 동일 액터가
    - 서버에서는 존재
    - 클라이언트 A에는 보이고
    - 클라이언트 B에는 보이지 않는 상황을 의도적으로 만들 수 있다
- “왜 안 보이는지”를 말로 설명 가능
- NetMode / Role / Relevancy 로그가 남아 있음