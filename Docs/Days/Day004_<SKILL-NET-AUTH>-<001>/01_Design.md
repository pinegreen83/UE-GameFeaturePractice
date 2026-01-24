## Goal
본 티켓의 목표는 스킬 실행 과정에서
입력 의도(Intent) → 서버 검증(Validation) → 결과 확정(Result) → 클라이언트 연출(Presentation)
의 책임을 명확히 분리하는 것이다.

클라이언트는 즉각적인 조작감을 위해 연출을 담당하되,
게임 상태에 영향을 미치는 모든 판정과 결과는 서버만이 결정하도록 하여
치트에 강하고, 멀티플레이 확장이 가능한 구조를 설계하는 것을 목표로 한다.

## Class / Component Responsibilities
Character
- 월드에 존재하는 플레이어/AI의 물리적 표현
- 입력을 직접 해석하거나 스킬 판정을 수행하지 않음
- 입력을 SkillComponent로 전달
- 서버 결과에 따른 애니메이션/연출을 수신

⸻

SkillComponent
- 캐릭터가 보유한 스킬의 관리 주체
- 클라이언트:
    - 입력 기반 SkillIntent 생성
    - 서버에 스킬 사용 요청(Server RPC)
    - 서버 결과에 따른 연출 재생 또는 실패 피드백 처리
- 서버:
    - SkillIntent 검증 로직 수행
    - 승인 시 실제 스킬 영향 실행
    - 결과를 클라이언트에 전파

※ 동일한 컴포넌트이지만
※ 판정 및 실행 로직은 서버 인스턴스에서만 수행

⸻

Server Logic (SkillComponent 내부)
- 스킬 실행에 대한 유일한 판정자
- 검증 항목:
    - 요청 권한 (Owner 여부)
    - 캐릭터 상태 (채널링, 경직, 사망 등)
    - 위치 및 월드 조건
    - 자원(스태미나/마나), 쿨타임
- 승인 시:
    - 데미지, 상태이상, 투사체 스폰 등 실제 영향 적용
    - SkillResult 생성 후 Broadcast
- 거절 시:
    - 요청 클라이언트에게만 거절 결과 전달

## Data Flow
1.	입력 발생 (Client)
- 플레이어 입력 수신
- SkillComponent에서 SkillIntent 생성
2.	의도 전달 (Client → Server)
- Server RPC를 통해 SkillIntent 전달
3.	검증 (Server)
- 현재 서버 기준 상태/월드 정보와 Intent 비교
- 스킬 사용 가능 여부 판단
4.	결과 생성 (Server)
- 승인 또는 거절 결정
- SkillResult 구조체 생성
5.	결과 전파
- 승인:
    - NetMulticast RPC로 모든 클라이언트에 결과 전파
- 거절:
    - Client RPC로 요청 클라이언트에게만 통지
6.	연출 처리 (Client)
- 승인:
    - 서버 결과 기반 애니메이션/이펙트 재생
- 거절:
    - 연출 미재생 또는 불발 피드백(UI/SFX)

## Edge Cases
- 서버 승인 전 클라이언트 연출이 재생된 경우
    - 거절 시 즉시 중단 또는 불발 처리 필요
- 클라이언트가 조작한 위치/방향 정보 전달
    - 서버는 해당 값을 그대로 신뢰하지 않고 재확정
- 중복 요청 (연속 입력, 네트워크 지연)
    - 서버에서 쿨타임/상태 기준으로 필터링
- 승인 결과 지연 도착
    - 클라이언트는 서버 결과를 항상 우선 적용

## Perf / GC / Tick Notes
- Tick 기반 판정 금지
    - 모든 스킬 실행은 이벤트 기반 처리
- SkillIntent / SkillResult는 POD 구조체로 설계
    - 동적 메모리 할당 최소화
- 연출 관련 오브젝트는 클라이언트 전용
    - 서버에는 불필요한 이펙트/사운드 생성 금지
- 서버 검증 로직은 최소 연산으로 유지
    - 중앙 큐나 글로벌 매니저 도입은 지양

## Replication Notes (선택)
- 서버 권한(Server Authority) 기반 구조
- 승인 결과는 NetMulticast RPC 사용
- 거절 결과는 Owner Client RPC 사용
- 데미지/상태 값은 서버에서만 변경 후 Replication
- 클라이언트의 예측 값은 서버 값 수신 시 항상 덮어씀