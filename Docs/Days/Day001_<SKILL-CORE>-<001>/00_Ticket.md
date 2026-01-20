[Task ID]

SKILL-CORE-001

[Title]

캐릭터 공통 Skill Execution Context 구조 설계 및 적용

[Background]

현재 개념상
- 플레이어와 몬스터는 동일한 스킬 데이터를 사용
- 차이는 “누가 조종하느냐(PlayerController vs AIController)” 뿐이다

하지만 스킬 실행 시점에서
- 소유자
- 타겟
- 입력 주체
- 네트워크 권한
이 뒤섞이기 시작하면,
스킬 로직이 캐릭터 타입별로 분기되기 쉬워진다.

이를 막기 위해 스킬 실행에 필요한 최소 정보 묶음(Context) 을 명시적으로 분리한다.

[Requirements]
- 스킬 실행 시 전달되는 Execution Context 구조체 정의
- Context에는 최소 다음 정보 포함
    - 스킬 소유자 (Owner)
    - 시전 주체 (Instigator)
    - 타겟 (단일 또는 없음 허용)
    - 월드 / 타이머 접근 가능 여부
- Player / AI 모두 동일한 인터페이스로 스킬을 실행할 수 있어야 함
- Skill 로직 내부에서
    - Player / AI 타입 체크 분기 금지

[Constraints]
- Character 파생 클래스에 직접 로직 몰아넣지 말 것
- Tick 기반 처리 금지 (필요 시 명시적 업데이트 함수)
- 네트워크 확장을 고려한 구조일 것 (Authority / Autonomous 개념을 망가뜨리지 않도록)
- “지금은 싱글플레이”라는 이유로 설계 단순화 금지
