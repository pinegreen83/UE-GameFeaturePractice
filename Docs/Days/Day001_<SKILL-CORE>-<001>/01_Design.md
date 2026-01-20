## Goal
Player / AI / 소환수 구분 없이 동일한 스킬 실행 파이프라인을 유지하기 위해, 스킬의 정적 데이터와 런타임 실행 정보를 분리하고 Controller는 입력 의도만 전달하며, SkillComponent가 실행 책임을 지고 Server가 최종 확정하는 구조를 설계한다. 이를 통해 네트워크 권위, 확장성, 유지보수를 동시에 만족하는 스킬 실행 구조를 확보한다.

## Class / Component Responsibilities
Controller (PlayerController / AIController)
- 유저 입력 또는 AI 판단을 Intent 데이터로 변환
- SkillId, 조준 정보(AimDir / Target / HitResult 등) 전달
- 실행 성공/실패에 대한 책임 없음
- 스킬 로직/쿨다운/데미지 계산 관여 금지

SkillComponent (Owner Actor에 부착)
- 스킬 실행의 중앙 책임자
- Intent를 받아 Candidate Execution Context 조립
- Owner, Instigator, Target 정보 구성
- 서버/클라 공통 인터페이스 유지
- 서버에서는 자원/쿨다운/타겟 검증 후 실행 확정
- 클라이언트에서는 예측 연출(애니/이펙트) 가능
- 쿨다운 상태 및 스킬 사용 가능 여부 관리

Server (Authoritative)
- SkillComponent의 Server RPC를 통해 요청 수신
- Candidate Context 검증 및 Authoritative Context 확정
- 자원 소모, 쿨다운 EndTime 확정
- 데미지 / 상태이상 / 회복 적용
- 실행 결과를 Replication 또는 Multicast로 전파

Skill Data (USTRUCT 기반)
- 스킬의 정적 정의 정보만 포함
- 공통 헤더(이름, 설명, 썸네일)
- Active / Passive Spec
- 런타임 상태(Owner, Instigator, 타이머 등) 절대 포함하지 않음
- DataTable 또는 Asset 형태로 관리

## Data Flow
1.	입력 단계
- PlayerController / AIController가 스킬 사용 의도를 생성
2.	요청 단계
- Controller → SkillComponent로 Intent 전달
3.	Context 조립
- SkillComponent가 Candidate Execution Context 생성
4.	서버 검증
- Server RPC 호출
- 자원 / 쿨다운 / 타겟 검증
- Authoritative Context 확정
5.	실행 및 적용
- 서버에서 데미지, 상태이상, 쿨다운 반영
- 필요 시 Projectile / Area Actor 생성
6.	결과 전파
- Replication / Multicast로 애니메이션 및 이펙트 동기화
- 클라이언트 UI는 쿨다운 EndTime 기반으로 표시

## Edge Cases
- 클라이언트 예측 실행 후 서버 거절(자원 부족, 쿨 미완료)
- 타겟이 서버 검증 단계에서 유효하지 않은 경우
- Controller 교체(리스폰, 빙의 변경) 시 책임 주체 유지
- AI와 Player가 동일 스킬을 사용할 때의 인터페이스 일관성
- 패시브 스킬처럼 실행 이벤트가 없는 경우의 처리 경로

## Perf / GC / Tick Notes
- Execution Context는 USTRUCT로 관리하여 GC 부담 최소화
- 스킬 자체를 Actor로 생성하지 않음
- Projectile / Area 등 월드 오브젝트만 Actor 사용
- Tick 기반 스킬 로직 금지
- 쿨다운은 EndTime + TimerManager 기반 이벤트 처리
- SoftObjectPtr 사용으로 초기 메모리 사용 최소화

## Replication Notes (선택)
- 스킬 사용 요청은 Server RPC 기반
- 결과 동기화는 RepNotify 또는 Multicast 사용
- 쿨다운은 서버 기준 EndTime 복제
- 데미지 및 상태이상은 서버 전용 처리
- 예측 연출은 클라이언트 로컬 처리 가능하되, 서버 결과로- 