[Task ID]

SKILL-STATE-001

[Title]

스킬 실행을 상태 머신(State)으로 분리하고 Tick 없는 갱신 구조 설계

[Background]

이전 티켓에서는
- 입력 → 의도(Intent) 생성
- 서버 확정
- 컴포넌트 책임 분리
까지 다뤘다.

이제 다음 단계는 “실행 중인 스킬을 어떻게 안전하게 관리할 것인가”다.
현업에서는 스킬/행동을 bool이나 enum으로 흩뿌리지 않고,
명시적인 State 객체로 관리하는 경우가 많다.

[Requirements]
- 스킬 실행 흐름을 State 개념으로 모델링
    - 예: Idle / Casting / Executing / Cooldown
- State는 데이터 + 책임을 함께 가짐
- State 전환은 명확한 조건을 통해서만 발생
- Tick 기반 갱신 ❌
    - Timer / Delegate / 명시적 Update 호출 구조 사용

[Constraints]
- ASandboxCharacter는 상태를 직접 판단하지 않는다
- 상태 전환 로직은 SkillComponent (또는 유사 책임 클래스)에 위치
- GC 안전성 고려 (UObject vs Struct 선택 이유 명확히)
- 네트워크 확장을 고려한 구조여야 함
(지금은 싱글 기준, 하지만 “서버 authoritative”로 확장 가능해야 함)