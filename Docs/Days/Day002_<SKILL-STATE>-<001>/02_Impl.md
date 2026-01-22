## Implementation Summary
이번 티켓(SKILL-STATE-001)은 **스킬 실행을 FSM(Idle/Casting/Executing/Cooldown)**으로 관리하고, 
조작 제한은 별도 ControlStateComponent에서 합성(Reason) 후 Controller에 적용하는 구조로 구현했다.
Tick 없이 Timer(Casting/Cooldown)와 Montage delegate(Executing 종료)로 전이를 수행하며, 
전이는 항상 Exit -> Enter 순서로 처리하여 정리(Reason handle 해제)가 전이 경로에 관계없이 보장되도록 했다.

## Files
- Tickets/Day002-SKILL-STATE-001/ControlStateComponent_SK_ST_001.h/.cpp
- Tickets/Day002-SKILL-STATE-001/SkillComponent_SK_ST_001.h/.cpp
- SandboxCharacter.cpp (스킬 입력 바인딩 및 TryStartSkill() 호출)

## Core Logic
1) ControlStateComponent: Reason 합성 → Controller 적용
- AddBlockReason()에서 ReasonCounts 증가 및 핸들 반환
- RemoveBlockReason()에서 핸들 기반으로 감소/정리
- RecomputeAndApply()에서 OverrideState 우선, 없으면 ReasonCounts 기반으로 FControlSnapshot 계산
- ApplyToPawn()에서 최종 결과를 Controller->SetIgnoreMoveInput / SetIgnoreLookInput로 적용

핵심 의도:
- “스킬이 직접 이동/입력을 막는다”가 아니라 스킬은 Reason를 추가/제거(정보 제공)하고,
ControlComponent가 최종 적용(권한)을 가진다.

2) SkillComponent: FSM + Tick 없는 전이 + Exit 정리 보장
- TryStartSkill() : Idle에서만 Casting으로 전이
- Casting:
    - MoveLock/InputLock Reason 추가
    - CastingTimer로 Executing 전이 트리거
- Executing:
    - Montage_Play()
    - OnMontageEnded / OnMontageBlendingOut 델리게이트 바인딩
    - 종료 시 Cooldown 전이
- Cooldown:
    - CooldownTimer로 Idle 복귀
- ExitState():
    - 타이머 클리어
    - 몽타주 델리게이트 언바인딩
    - Reason handle 해제(전이 사유와 무관하게 공통 정리)

3) Logging
디버깅/학습을 위해 상태 전이 및 Reason 합성 흐름을 로그로 관찰할 수 있도록 추가했다.
- Skill: TryStartSkill / Transition / Enter / Exit / Montage callback
- Control: AddReason / RemoveReason / Snapshot / Apply

## Design Match Check
- ✅ “ASandboxCharacter는 상태를 직접 판단하지 않는다”
→ Character는 입력을 받아 SkillComponent의 TryStartSkill()만 호출.
- ✅ “상태 전환 로직은 SkillComponent에 위치”
→ FSM(Idle/Casting/Executing/Cooldown), 전이 트리거(타이머/델리게이트)가 SkillComponent에 집중.
- ✅ “Tick 없는 갱신”
→ 타이머 + 몽타주 델리게이트로만 전이.
- ✅ “조작 제한은 bool toggle이 아니라 합성(Reason)”
→ ControlStateComponent가 ReasonCounts로 최종 CanMove/CanLook/CanInput을 산출.
- ⚠️ 설계 대비 구현 차이/주의:
    - SetIgnoreMoveInput은 “MoveAxis 기반 이동” 위주로 차단하며, Jump 같은 액션 입력은 별도로 차단되지 않을 수 있다(현상 관찰됨).
    - Casting→Executing 전이 시점에 Exit(Casting)에서 Reason를 해제하여, Executing 동안 잠금이 유지되지 않는 구성(“어느 상태에서 유지할지”를 상태별로 조정 필요).
    - AnimInstance에서 Snapshot을 읽어 AnimGraph에 반영하는 코드는 아직 추가하지 않았고, 현재는 “제어/전이 구조 학습” 중심 구현.

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.