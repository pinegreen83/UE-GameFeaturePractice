## What Worked
- 책임 분리 체감이 명확함
    - SkillComponent는 “스킬 실행 수명주기(FSM)”에만 집중
    - ControlStateComponent는 “조작 가능 여부 계산 + Controller 적용”으로 일원화
    - 결과적으로 “누가 잠그고/푸는가”가 한 곳에 모여 디버깅이 쉬움
- Tick 없는 전이 구조가 잘 동작
    - Casting/Cooldown은 Timer, Executing 종료는 Montage delegate로 처리되어 요구사항 충족
- Exit 정리(Handle 해제) 패턴이 안정적
    - 어떤 이유로 전이가 발생하더라도 Exit에서 공통 정리를 수행 → 잠금 누락/중복 해제 문제를 크게 줄임
- 로그를 통해 흐름이 관찰 가능
    - 전이(0→1→2→3→0) 및 Reason 합성 스냅샷이 콘솔에서 확인됨

## What Broke
- 입력 중복으로 TryStartSkill 로그가 2회 찍히는 현상
    - 실제로 같은 트리거가 중복 발화하거나 바인딩/컨텍스트가 중복 적용됐을 가능성
    - 학습 단계에서 “왜 2번 호출되는지”가 혼선을 유발
- MoveLock이 ‘완전한 이동 봉쇄’가 아님
    - SetIgnoreMoveInput은 이동축에는 강하지만 Jump 같은 액션 입력을 자동으로 막지 않음
    → “걷기 막힘 / 점프(혹은 달리기) 가능” 같은 현상 발생
- 잠금 유지 범위가 의도와 다르게 짧게 적용될 수 있음
    - Casting에서 Add한 MoveLock/InputLock이 Casting Exit에서 해제되어 Executing 동안 잠금이 유지되지 않음
    → “채널링 동안만 묶이고 모션 중엔 풀리는” 형태가 됨

## Fix / Improvement
- 로그 정리(가독성 개선)
    - Exit 로그 중복 출력 제거(한 번만 또는 Before/After로 명확화)
    - RemoveReason 로그는 감소 전이 아니라 감소 후(또는 Before/After)로 변경하여 혼란 제거
- 입력 중복 진단
    - UseSkill()에 타임스탬프 로그 추가하여 동일 프레임 2회 호출 여부 확인
    - 중복이면 MappingContext 중복 추가, BP 바인딩 중복, Trigger 설정 확인
- “완전 묶기”가 필요할 때의 러프 해법(학습용)
    - MoveLock 적용 시점에 DisableMovement() / 해제 시 SetMovementMode(MOVE_Walking)로 “점프 포함 전면 봉쇄”를 체감 가능
    - 단, 이 방식은 실무에선 충돌 여지가 있어 이후 단계에서 입력 게이트(Controller/EnhancedInput)로 교체 권장
- 잠금 유지 범위 명확화
    - MoveLock/InputLock을 Casting+Executing 동안 유지하려면 “Exit(Casting)에서 제거하지 말고 Exit(Executing)에서 제거” 같은 상태별 해제 정책이 필요

## Next Step
- Cancel 입력 바인딩 추가
    - RequestCancel(ByPlayerInput)를 호출해 “전이 + Exit 정리”가 실제로 안전하게 동작하는지 확인
- 입력 게이트를 Controller로 이동(선택)
    - Jump/Skill/Cancel 같은 액션 입력을 Controller에서 스냅샷(bCanInput)으로 early-return 처리하면 제어가 일관됨
- AnimInstance Pull 연결(선택)
    - AnimInstance가 SkillState/ControlSnapshot을 읽어 AnimGraph에서 “스킬 중 상체/전신 블렌딩” 등 표현 확장 가능
- (확장) 서버 authoritative 고려
    - TryStartSkill/RequestCancel을 “요청”으로 보고, 서버 확정 후 상태 전이를 복제/동기화하는 형태로 확장 가능