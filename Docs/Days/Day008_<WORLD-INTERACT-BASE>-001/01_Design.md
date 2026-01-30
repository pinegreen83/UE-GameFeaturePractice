## Goal
본 티켓의 목표는 멀티플레이 환경에서 월드 오브젝트 상호작용의 기준을 명확히 분리하는 것이다.
게임 내 모든 의미 있는 상태 변화는 의도를 가진 주체(플레이어 또는 AI) 가 생성한 요청을 통해서만 발생해야 하며, 해당 요청은 반드시 서버의 검증과 승인을 거쳐야 한다.
반면, 월드 상태를 변경하지 않는 근접 UI 및 시각적 피드백은 클라이언트 로컬에서 자유롭게 처리할 수 있도록 설계하여, 네트워크 부하와 구조 복잡도를 최소화한다.

## Class / Component Responsibilities
PlayerController / Character
- 입력 및 이동을 통해 의도(Intent)를 생성하는 주체
- 근접 오브젝트에 대한 Overlap을 감지하여:
    - 상호작용 가능 UI / 하이라이트 표시 (Client-only)
- 실제 월드 상태 변화를 요구하는 경우:
    - Server_RequestInteract(TargetActor) 형태로 서버에 요청 생성

Interactable / Pickup Actor
- 의도 생성 책임 없음
- 서버 승인 이후에만 자신의 상태를 변경
    - 아이템 소유권 변경
    - 파괴 / 비활성화
    - 문 열림 / 닫힘
- 동시 접근에 대한 Lock / Once 처리 책임

Server (Authority)
- 모든 상태 변화 요청의 최종 판단자
- 요청에 대해 다음을 검증:
    - 거리 / 위치
    - 플레이어 상태 (Dead, Stun 등)
    - 오브젝트 상태 (이미 사용됨, Lock 상태)
- 승인 시에만 오브젝트의 상태 변화 실행
- 결과를 Replication으로 전파

## Data Flow
1) 상태 변화를 수반하지 않는 Overlap (UI 전용)
    1.	Client Character가 오브젝트와 Overlap
    2.	클라이언트에서 UI / 하이라이트 표시
    3.	서버 통신 없음

2) 입력 기반 상호작용 (문, 아이템, 레버 등)
    1.	Client에서 Overlap로 상호작용 후보 인지
    2.	플레이어 입력 발생
    3.	Client → Server_RequestInteract(Target)
    4.	Server에서 검증
    5.	승인 시 오브젝트 상태 변경
    6.	결과 Replication

3) 자동 처리 오브젝트 (힐킷, 트리거 등)
    1.	Server에서 Overlap 또는 거리 조건 확인
    2.	서버가 직접 실행 여부 판단
    3.	오브젝트 상태 변경
    4.	결과 Replication

## Edge Cases
- 두 플레이어가 동시에 동일한 아이템을 획득 시도
    - 서버에서 최초 1회만 승인
- 클라이언트 UI는 표시되었으나 서버 검증 실패
    - 거리 초과 / 이미 사용됨
- 클라이언트 Overlap과 서버 Overlap 결과 불일치
    - 서버 판단을 항상 우선
- 자동 트리거의 중복 실행
    - Once 플래그 또는 Cooldown 필요

## Perf / GC / Tick Notes
- Overlap을 통한 UI 후보 관리는 클라이언트 전용으로 제한
- 서버는 입력 요청 또는 명확한 트리거 조건에서만 처리
- Tick 기반 거리 체크 지양
    - 필요 시 이벤트 기반 또는 단발 검증
- 오브젝트 수 증가 시에도 서버 요청 빈도는 입력 기준으로 제한됨

## Replication Notes (선택)
- 오브젝트의 상태 변화만 Replicate
- 클라이언트 UI / 하이라이트는 Replication 대상 아님
- 서버 승인 전 클라이언트에서 상태를 선반영하지 않음
    - 예측 처리 최소화