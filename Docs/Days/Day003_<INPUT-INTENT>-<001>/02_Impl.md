## Implementation Summary
- PlayerController 주도 방식으로 DefaultIMC를 로컬 플레이어 서브시스템에 적용했고, 콘솔 커맨드로 리바인드/덤프를 실행할 수 있게 구성했다.
- 런타임에서 IMC 매핑 배열을 직접 수정하려 시도했으나, UE5.4 환경에서 GetMappings()가 const로만 제공되어 원본 매핑 변경이 불가한 케이스를 확인했다.
- PlayerMappable 경로 또한 프로젝트/환경 조건 상 즉시 적용이 어려워, 현 티켓에서는 리바인드 구현을 “완료”보다 “경계 확인”으로 정리하고, 다음 단계에서 Intent/Resolver 설계로 이동하기로 결정했다.

## Files
- Tickets/Day003-INPUT-INTENT-001/SBPlayerController_SK_IN_001.h/.cpp
    - IMC 적용, 콘솔 커맨드(리바인드/덤프), 입력 바인딩 테스트
- Tickets/Day003-INPUT-INTENT-001/InputRebindComponent_SK_IN_001.h/.cpp
    - RebindActionKey / LoadAndApply / ReplaceKeyInIMC 시도

## Core Logic
(1) IMC 적용
- BeginPlay에서 UEnhancedInputLocalPlayerSubsystem에 DefaultIMC를 적용(AddMappingContext)

(2) 콘솔 커맨드 기반 리바인드
- SM_RebindAttackKey F 같은 형태로 실행
- 문자열을 FKey로 변환 후 RebindComponent로 전달

(3) 덤프
- SM_DumpBindings로 DefaultIMC의 Action-Key 목록 출력
    → 실제로 매핑이 변경되었는지 “눈으로” 확인하려는 목적

(4) 실패/제약 확인 포인트
- ReplaceKeyInIMC에서 IMC 매핑을 직접 수정하려 했지만,
    - GetMappings()가 const로 노출되어 TArray<FEnhancedActionKeyMapping>& 참조로 받을 수 없음
    - 결과적으로 “로그상 OK”처럼 보여도, 실제 덤프에서는 키가 바뀌지 않는 상태가 발생

## Design Match Check
- 일치한 부분
    - “IMC 적용은 한 곳(PC)에서만”이라는 책임 분리 방향은 정리됨
    - InputAction을 “행동 축”으로 유지한다는 설계 원칙은 유지됨
- 달라진/미완성 부분
    - 런타임 리바인드를 IMC 내부 배열 수정으로 처리하는 방식이 UE5.4에서 막힐 수 있어, 설계 자체를 “Subsystem 기반(PlayerMappable 등)”으로 바꿔야 함
    - 본 티켓에서는 리바인드를 완성하지 않고, 제약을 확인한 시점에서 중단

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.