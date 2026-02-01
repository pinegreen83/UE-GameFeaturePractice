## Implementation Summary
본 티켓에서는 스킬 실행 결과(성공/실패)를 UI/디버그와 분리된 피드백 채널로 전달하는 구조를 구현했다.
서버는 스킬 승인 파이프라인에서 결과만 확정하고, 클라이언트에서는 GameInstanceSubsystem을 통해 해당 결과를 수신하여 디버그 출력(Sink)으로 전달한다.
UI 구현 자체는 최소화하고, 이벤트 기반 구조의 뼈대를 만드는 데 집중했다.

## Files
- SandboxSkillTypes.h
    - FSandboxFeedbackEvent, ESandboxFeedbackResult, ESandboxFeedbackStage 정의
- FeedbackSubsystem_UI_FE_001.h / .cpp
    - 클라이언트 로컬 피드백 허브 (UGameInstanceSubsystem)
- FeedbackDebugComponent_UI_FE_001.h / .cpp
    - 피드백 이벤트를 수신하여 로그/온스크린 출력
- SkillComponent_UI_FE_001 (기존 스킬 컴포넌트 확장)
    - 서버 승인 결과를 FeedbackEvent로 변환하여 전달

## Core Logic
- 서버
    - 스킬 승인 파이프라인에서 실패 시 즉시 Reason 확정 후 Client RPC
    - Commit 성공 시에만 Success 이벤트 전송
- 클라이언트
    - 서버로부터 받은 결과를 FeedbackSubsystem에 Publish
    - Subsystem은 델리게이트를 통해 연결된 Sink로 이벤트 전달
- Debug Sink
    - OnFeedback 델리게이트를 구독
    - Result / Stage / SkillId / Reason을 로그 및 OnScreenDebugMessage로 출력

## Design Match Check
- ✔ 설계에서 정의한 “서버는 결과만 확정, 표현은 클라이언트 책임” 구조 유지
- ✔ UI를 직접 구현하지 않고, Sink 기반 확장 가능 구조로 구현
- ⚠ 현재 Debug Sink가 캐릭터에 부착되어 있어, 리슨 서버 환경에서는 중복 바인딩 가능성 존재

    → 설계 의도상 허용하되, 추후 위치 조정 예정

## Commit
- 로컬 실험용 구현 (단일 티켓 연습)
- Feedback 채널 구조 검증 목적
- 추후 Shared 구조 통합 시 별도 커밋 예정