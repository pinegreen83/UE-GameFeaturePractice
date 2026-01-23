[Task ID]

SKILL-NET-AUTH-001

[Title]

스킬 실행의 서버 권한(Auth) 분리 및 승인 흐름 설계

[Background]

지금 구조는
- 입력 → 의도(Intent) 생성
- 스킬 코어가 실행 판단

까지는 잘 되어 있음.
하지만 “누가 최종 실행을 결정하는가”에 대한 책임은 아직 명확히 분리되지 않음.

실무(특히 멀티플레이)에서는

클라이언트는 요청만 하고,
서버가 승인하고,
결과만 브로드캐스트

이 흐름이 기본이다.

이번 티켓은 Replication 자체 구현이 목적이 아니라,
👉 권한 분리 사고방식과 실행 책임 위치를 익히는 게 핵심이다.

[Requirements]
- 클라이언트는 SkillIntent만 생성하고 직접 실행하지 않는다
- 서버가 SkillIntent를 받아 유효성 검증 후 실행 여부 결정
- 실행 승인/거절 결과를 명확히 분리된 구조로 표현
- Character / SkillComponent / Server Logic 간 역할 경계가 코드로 드러나야 함
- “지금은 싱글이어도, 멀티를 고려한 구조”일 것

[Constraints]
- 실제 네트워크 RPC 구현은 최소화 (Mock 또는 함수 레벨로 표현해도 됨)
- Tick 기반 판정 금지
- AnimInstance가 실행 판단 주체가 되면 ❌
- 기존 티켓(STATE / CORE / INTENT) 구조를 침범하지 말고 확장할 것
- “일단 되게”보다 책임 분리가 우선