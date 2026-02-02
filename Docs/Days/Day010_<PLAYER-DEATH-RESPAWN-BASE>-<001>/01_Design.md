## Goal
전투 결과로 발생하는 사망(Death) 을 서버 권한 기반으로 단일 결정하고,
상태 변화는 델리게이트 기반 이벤트 흐름으로 전파하여
입력 차단, 연출, UI 등이 서로 직접 호출 없이 반응하도록 설계한다.

## Class / Component Responsibilities
Character
- 월드에 존재하는 엔티티
- 위치/회전/애니메이션 표현 담당
- 내부 컴포넌트들의 라우팅 허브
- 사망/리스폰 연출은 상태 변화에 반응해서 처리

StatComponent
- HP 등 수치 데이터의 소유 및 변경 책임
- 서버 커밋 시 HP Delta 적용
- HP 변경 사실을 델리게이트로 통지
    - 예: OnHealthChanged, OnHealthDepleted
- 상태 전환 결정은 하지 않음

State (LifeState) Component
- 캐릭터의 행동 가능 여부와 라이프 사이클 규칙 책임
- 서버에서만 상태 전환 결정
    - Alive → Dead
- StatComponent의 이벤트를 구독하여
    - HP ≤ 0 감지 시 LifeState = Dead 전환
- 상태 변경 시 델리게이트 발행
    - OnLifeStateChanged(Prev, New)

PlayerController
- 입력 해석 및 IMC 적용 책임
- Pawn 빙의 시 StateComponent의 상태 변경 델리게이트를 바인딩
- Dead 상태 진입 시
    - 입력 차단 / IMC 제거
- Alive 복귀 시 입력 복구

HUD / UI
- StateComponent의 상태 변경 델리게이트를 구독
- Dead 상태에서 사망 UI 표시
- 리스폰 시 UI 복구

## Data Flow
1.	Client 공격 입력
2.	Client에서 의도(Intent) 생성 → Server RPC
3.	Server 검증 → 커밋
4.	커밋 과정에서 StatComponent가 HP 변경
5.	StatComponent가 HP 변경 이벤트 델리게이트 발행
6.	StateComponent가 해당 이벤트를 수신
    - (Authority일 때만) HP ≤ 0이면 LifeState = Dead
7.	LifeState 변경 → RepNotify
8.	StateComponent가 OnLifeStateChanged 델리게이트 발행
9.	구독자 반응
    - Controller: 입력 차단
    - Character/Anim: 사망 모션
    - HUD: 사망 UI

    상태 변화는 단일 결정 → 다수 반응 구조로 흐름이 전파됨

## Edge Cases
- HP가 0 이하로 여러 번 내려가는 경우
    - LifeState != Dead 가드로 사망 처리 1회 보장
- 클라이언트 임의 HP 변경 시도
    - 서버 커밋 없이는 상태 전환 불가
- 리스폰/재빙의 시
    - 델리게이트 중복 바인딩 방지
    - OnUnPossess / EndPlay에서 정리 필요
- UI가 Pawn보다 늦게 생성되는 경우
    - 바인딩 시 현재 상태 동기화 필요

## Perf / GC / Tick Notes
- Tick 기반 FSM 사용하지 않음
- 상태 변화는 이벤트 기반
- 델리게이트 바인딩은 BeginPlay / OnPossess 시점에 1회
- Dynamic Delegate는 UI(BP) 노출이 필요한 경우에만 사용
- 불필요한 폴링/분기 제거로 성능 안정성 확보

## Replication Notes (선택)
- LifeState는 ReplicatedUsing=OnRep_LifeState
- 서버에서만 상태 변경
- OnRep에서 동일한 델리게이트를 호출해
    - 서버/클라 반응 경로 통일
- 향후 Spectator / Team Respawn 확장 가능