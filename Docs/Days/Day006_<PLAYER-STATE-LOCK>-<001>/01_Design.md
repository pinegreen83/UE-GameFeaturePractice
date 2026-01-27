## Goal
Enhanced Input(Action/IMC)로 들어온 입력을 얇은 콜백(어댑터) 로 수신한 뒤, 
프로젝트 레이어의 InputGate에서 “공통 정책(상태 기반 입력 허용/차단)”만 중앙에서 판정하고, 
허용된 입력만 해당 행동 컴포넌트(이동/전투/스킬 등) 로 라우팅한다.
행동 고유 검증(쿨타임/거리/타겟/자원 등)은 각 행동 시스템의 책임으로 유지하여 Gate 비대화를 방지한다.

## Class / Component Responsibilities

APlayerController (입력 주관, 로컬)
- Enhanced Input 바인딩 소유 (IMC 추가/우선순위 관리 포함)
- Action 콜백에서 로직/상태 판단 금지
- 콜백은 InputGate로 “Verb + EventType(+ Value)”를 전달하는 1~2줄 라우팅만 수행

UInputGateComponent (로컬 입력 정책/라우팅)
- 공통 정책(Policy): 캐릭터 상태를 조회해서 “이 입력을 지금 소비 가능한가”만 판단
- 라우팅(Dispatch): 허용된 입력을 대상 컴포넌트(예: Combat/Skill/Interact)로 전달
- 입력 거부 시: 로그 또는 델리게이트로 “Rejected(Verb, Reason)”만 발행 (콜백이 해석하지 않음)
- 하지 않는 일: 쿨타임/자원/거리/타겟 등 행동 고유 검증, 실제 스킬 실행 로직

    위치: “입력의 주관”을 유지하려면 PC 소유가 자연스럽고, 판단 근거는 Character 상태 조회로 가져간다.

ASandboxCharacter (상태/데이터 소유, 서버 기준)
- “입력 가능/불가의 근거가 되는 상태”의 소유자
- 상태/스탯/스킬 등은 컴포넌트로 분리하고, Gate는 이를 조회만 한다

UStatusComponent (서버 authoritative)
- Stun/Silence/UI Lock 같은 공통 제약 상태를 관리(추가/제거/복제)
- Gate가 참조하는 CanConsume(Verb, EventType) 또는 “현재 제약 집합”을 제공
- 네트워크에서는 서버가 최종 상태를 확정하고(Replication), 클라는 UX용으로만 참고 가능

행동 컴포넌트 (예: USkillComponent / UCombatComponent / UInteractComponent)
- 허용된 입력을 받아 실제 행동 수행
- 행동 고유 검증(쿨타임/자원/타겟 등)은 여기서 처리
- 멀티플레이에서는 서버 RPC/Authority 흐름에 맞춰 최종 실행 확정

## Data Flow
1.	Enhanced Input
- IMC 우선순위/트리거/모디파이어를 통해 “의미 있는 Action 이벤트” 생성

2.	PlayerController 콜백(어댑터)
- OnMove(Value) / OnAttack(Started) 같은 콜백은 로직 없이
- InputGate->Request(Verb, EventType, ValueOptional) 형태로 전달

3.	InputGate
- StatusComponent를 조회해 공통 정책으로 허용/차단 판정
- 허용: Dispatch(Verb, EventType, Value)로 행동 컴포넌트에 전달
- 차단: Rejected(Verb, Reason) 로그/이벤트 발행

4.	행동 컴포넌트
- 허용된 요청을 받아 행동 수행
- 행동 고유 검증은 이 단계에서 처리
- 필요 시 서버에 요청 → 서버가 최종 실행/실패 확정

## Edge Cases
- Gate 또는 StatusComponent가 없을 때: 크래시 없이 “N/A/Rejected” 처리(로그)
- Possess/UnPossess 전환 시: 바인딩 및 Gate 참조가 끊기지 않도록 생명주기 관리
- 입력 이벤트 중 Canceled(홀드/차지/채널링 취소) 처리 누락 방지
- 서버/클라 상태 불일치:
    - 클라는 로컬 상태로 1차 UX 필터 가능
    - 서버는 최종 실행 단계에서 반드시 재검증(치트/지연 대비)

## Perf / GC / Tick Notes
- Tick 기반 입력 판정 금지 (입력 이벤트 기반)
- Gate는 “간단한 상태 조회 + 라우팅”만 수행하여 호출 비용 최소화
- 델리게이트 사용 시 강참조/순환참조 주의(특히 UMG 바인딩)

## Replication Notes (선택)
- “상태(제약)”는 Character/StatusComponent에서 서버 authoritative로 복제
- “입력 수신”은 로컬 전용(PC)이며 복제 대상이 아님
- 행동 실행은 기존 네 구조(서버 승인/검증/실행) 흐름과 동일하게 서버에서 최종 확정