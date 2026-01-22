[Task ID]

INPUT-INTENT-001

[Title]

입력 → 의도(Intent) 변환 계층 설계 및 적용

[Background]

지금까지는
- 스킬 상태 관리 (STATE)
- 스킬 실행 코어 (CORE)

에 집중했다면, 이제 “입력 그 자체”와 “게임이 이해하는 행동”을 분리할 타이밍이야.

실무에서는 키 입력 = 행동이 아니라
키 입력 → 의도(Intent) → 행동/스킬/상태 구조가 기본이야.

[Requirements]
- Enhanced Input으로 들어온 입력을 직접 스킬에 연결하지 않는다
- FInputIntent (USTRUCT) 정의
    - 방향 벡터
    - 입력 타입 (Pressed / Released / Hold 등)
    - 입력 발생 시점(Time)
- Character는 입력을 해석하지 않고 Intent만 생성
- Skill / State / Controller는 Intent를 소비(consumption) 하는 구조로 설계
- 로그로 Intent 생성 → 소비 흐름 추적

[Constraints]
- Tick 기반 처리 금지 (이벤트 기반)
- Input ↔ Skill 직접 참조 금지
- Intent는 값 타입(Struct)으로만 전달
- 네트워크 확장 가능성 고려 (Client 생성 / Server 검증 구조 염두)