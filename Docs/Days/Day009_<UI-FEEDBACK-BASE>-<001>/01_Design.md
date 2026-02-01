## Goal
본 티켓의 목표는 스킬 실행 파이프라인에서 발생하는 최종 결과(성공/실패)를 표준화된 이벤트 형태로 클라이언트에 전달하는 구조를 확립하는 것이다.
UI를 직접 구현하는 것이 아니라, 서버에서 확정된 결과를 표현 레이어(UI, 디버그 로그 등)와 분리된 피드백 채널을 통해 전달함으로써, 
이후 UI/로깅/디버깅 시스템을 유연하게 확장할 수 있는 기반을 마련한다.


## Class / Component Responsibilities
Server Skill Logic (기존 Day004 기반)
- 스킬 요청에 대한 권한 검증, 타당성 검토, 적용, Commit 책임 유지
- 각 실패 지점에서 실패 Reason을 확정
- 성공 시 Commit 단계에서만 성공 결과 확정
- 결과를 클라이언트로 전송하는 책임까지만 가짐 (표현 X)

Feedback Event (Data)
- 스킬 실행 결과를 표현하는 순수 데이터
- 성공/실패 여부와 실패 사유를 enum 형태로 보유
- UI 문구, 색상, 연출 정보는 포함하지 않음

Feedback Subsystem (Client)
- 클라이언트 로컬에 존재하는 결과 수신 및 분배 채널
- 서버로부터 전달된 FeedbackEvent를 받아 델리게이트로 브로드캐스트
- UI, 디버그 로그 등 여러 sink를 연결 가능
- 게임플레이 로직에는 관여하지 않음

UI / Debug Sink (Client)
- FeedbackSubsystem을 구독
- Reason enum을 기준으로 FText 조립 및 출력 방식 결정
- 현재 티켓에서는 OnScreenDebugMessage + UE_LOG만 사용

## Data Flow
1.	Client에서 스킬 입력 발생 → Intent 생성
2.	Intent를 Server로 전송
3.	Server에서 스킬 검토 파이프라인 수행
    - 검토 / 타당성 확인 / 적용 / Commit 단계
4.	각 단계에서 실패 시:
    - 실패 Reason 확정
    - 즉시 해당 Client에게 FeedbackEvent 전송
    - 이후 단계 진행 없이 종료
5.	Commit 단계까지 성공 시:
    - 성공 FeedbackEvent를 1회 전송
6.	Client는 FeedbackEvent 수신
7.	FeedbackSubsystem이 이벤트 브로드캐스트
8.	연결된 sink(UI/디버그 로그)가 결과를 표시

## Edge Cases
- 클라이언트 예측 애니메이션이 실행된 상태에서 서버 실패 발생

    → 애니메이션 여부와 무관하게 서버 결과만을 기준으로 Feedback 처리
- 동일 프레임 내 다중 실패 발생 가능성

    → 가장 먼저 발생한 실패 Reason만 전송
- 실패 이벤트를 Multicast로 전송할 경우

    → 다른 플레이어에게 불필요한 피드백 노출 위험

    → 실패는 Owner Client 전용 전송을 기본으로 함
- UI 제거 또는 변경 시

    → FeedbackSubsystem 및 서버 로직 수정이 발생하지 않아야 함

## Perf / GC / Tick Notes
- FeedbackSubsystem은 이벤트 기반 구조로 Tick 사용 없음
- FeedbackEvent는 일회성 값 타입 데이터로 GC 부담 없음
- 실패 시 즉시 return 구조를 유지하여 불필요한 후속 연산 방지
- 디버그 출력은 개발 환경에서만 활성화 가능하도록 분리 가능

## Replication Notes (선택)
- 스킬 실행 결과는 서버 권한 기준으로만 확정
- 실패 FeedbackEvent는 기본적으로 Owner Client RPC
- 성공 FeedbackEvent는 연출 공유가 필요한 경우에만 Multicast 고려
- FeedbackSubsystem 자체는 네트워크 개념을 알지 않으며, 네트워크 책임은 Server Skill Logic에 한정