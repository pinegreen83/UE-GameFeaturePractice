[Task ID]

WORLD-INTERACT-BASE-001

[Title]

플레이어가 월드 오브젝트와 상호작용할 수 있는 기본 인터랙션 파이프라인 구축

[Background]

지금 구조는 플레이어의 능동 행동(스킬, 입력) 은 잘 정리돼 있는데,
게임 세계가 플레이어에게 반응하는 구조는 아직 비어 있음.
- 상자 열기
- 레버 당기기
- NPC 상호작용
- 픽업 아이템

이 전부가 “스킬이 아닌 게임”의 핵심이고,
실무에선 여기 구조가 허술하면 콘텐츠 확장이 바로 막힘.

[Requirements]
- 플레이어는 월드의 특정 Actor와 상호작용할 수 있어야 한다
- 상호작용 대상은 스킬 시스템과 독립적이어야 한다
- 상호작용 가능 여부는 서버에서 최종 확정
- 실패 케이스:
    - 거리 부족
    - 이미 다른 플레이어가 사용 중
    - 상태상 상호작용 불가 (ex. Stun)

[Constraints]
- UE 5.4
- Tick 기반 거리 체크 ❌
- UInterface 기반 설계 필수
- 네트워크 확장 가능 구조 (지금은 단일 구현이라도 OK)
- Character가 오브젝트를 “직접 조작”하지 말 것

[Definition of Done]
- Character → Interaction 시도 → Server 승인 → Object 반응 흐름이 명확히 분리됨
- 상호작용 오브젝트 1종 (ex. TestChest, TestLever 등) 정상 동작
- 스킬 코드와 의존성 없음
- 로그만 보고도 흐름 설명 가능