## Goal
스킬 사용 요청의 단일 진입점(TryActivateSkill)을 기준으로, Resolve → Validate → Commit/Execute 3단계를 고정한다.
Validate는 SkillIntent(의도) 를 입력으로 받아 현재 캐릭터 상태/자원/쿨타임을 읽기 전용으로 검증하고, 
그 결과(SkillValidationResult)를 기반으로 서버가 승인/거절 및 실행을 최종 결정한다. 
이를 통해 “눌렀는데 왜 안 나갔는지”를 실패 이유로 설명 가능하게 만든다.

## Class / Component Responsibilities
Character
- 입력을 직접 판정하지 않음
- 입력을 SkillComponent로 전달(또는 PC → SkillComponent)
- 서버 결과에 따른 연출(애니/이펙트)은 결과 수신 후 처리

SkillComponent (핵심)
- 스킬 사용 요청의 단일 진입점 제공: TryActivateSkill(SkillId, Intent)
- 스킬 정의(가벼운 수치/규칙) 캐시 보유
- 스킬 런타임 상태(쿨타임 종료 시각 등) 보유
- 의존 컴포넌트 참조(예: StatComponent) 관리
- 서버에서만 Commit/Execute 수행 및 결과 전파

StatComponent
- 자원(스태미나 등) 조회/소모 API 제공
- 상태(예: 기절/경직/사망 등) 조회 API 제공(또는 별도 상태 컴포넌트로 분리 가능)
- 스킬 로직을 알 필요 없음(공개 API로만 상호작용)

## Data Flow
0) 초기화(캐시)
	1.	SkillComponent BeginPlay에서 DataTable/Asset로부터 스킬 정의를 로드하여 SkillId → SkillDef 캐시 구성
	2.	StatComponent는 BeginPlay에서 1회 찾아 TWeakObjectPtr로 캐시

1) 입력 → 의도 생성(Client)
	1.	입력 발생
	2.	SkillComponent가 SkillIntent 생성(방향/원점/타겟 정보 등 필요한 최소값)
	3.	Client는 Server_TryActivateSkill(SkillId, Intent) 요청

2) 서버 승인 흐름(Server)
	1.	서버 RPC 수신
	2.	TryActivateSkill(SkillId, Intent) 호출
        
        내부 단계:
        - (A) ResolveContext: Owner/StatComp/SkillDef 확보
        - (B) Validate(Intent): 읽기 전용 검증 → SkillValidationResult 생성
        - (C) Commit/Execute: Validate 성공 시 서버만 자원 소모/쿨타임 갱신/실행 적용
	3.	SkillResult 생성(성공/실패 + Reason + 필요 시 남은 쿨타임/부족 자원 값)
	4.	승인: NetMulticast(또는 Rep)로 결과 전파 / 거절: 요청자에게만 결과 전송

3) 결과 수신(Client)
	1.	승인 결과면 연출 재생(애니/이펙트/사운드)
	2.	거절 결과면 실패 이유 기반 피드백(UI/SFX)

## Edge Cases
- 연타/중복 요청: 동일 SkillId가 연속 요청될 때 서버 Validate에서 쿨타임으로 거절되며, Commit은 1회만 발생해야 함
- 자원 부족: Client에서 충분해 보였는데 서버에선 부족한 경우 → 서버 결과를 우선 적용(거절)
- 의존 컴포넌트 누락: StatComponent가 없거나 파괴된 경우 → MissingDependency로 즉시 거절
- Intent 데이터 신뢰 문제: Client가 전달한 위치/방향은 서버에서 필요 시 재확정(치트/오차 대비)
- 승인 지연: 클라가 먼저 연출을 재생했을 경우 → 거절 시 중단/불발 처리(현재 티켓에선 “예측 최소화” 권장)

## Perf / GC / Tick Notes
- Tick 기반 쿨타임 감소 금지
    - CooldownEndTime(서버 시간 기반) 비교로 판정
- DataTable 매 호출 조회 금지
    - BeginPlay에 스킬 정의 캐시
- 무거운 리소스(Anim/Niagara/Sound)는 SkillDef에 하드 레퍼런스 금지
    - ResourceId 또는 SoftObjectPtr로 분리
- StatComponent 참조는 약참조 + IsValid() 체크로 안전성 확보

## Replication Notes (선택)
- 최종 판정/Commit은 서버에서만 수행
- 성공 결과는 NetMulticast(또는 Rep)로 브로드캐스트
- 실패 결과는 Owner Client RPC로 단일 전송
- 런타임 상태(쿨타임 종료 시각 등)는 서버 권위가 기준이며, 필요 시 UI용으로만 최소 복제 고려