## Goal
SKILL-STATE-001의 목표는 "스킬 실행을 단일 상태 머신으로 모델링하고, Tick 없이 Timer/Delegate 기반으로 전이시키며, 
어떤 종료(정상 종료/캔슬/피격 인터럽트)에서도 Exit 경로로 수렴해 정리(해제/원복)가 100% 보장되는 구조"를 만드는 것이다.
AnimInstance는 상태를 '결정'하지 않고 읽기 전용으로 사용하며, 게임플레이 규칙/전이/잠금 결정은 SkillComponent와 ControlStateComponent가 담당한다.

## Class / Component Responsibilities

### ASandboxCharacter
- 입력을 직접 해석/판단하지 않는다.
- PlayerController/EnhancedInput에서 전달된 입력을 SkillComponent에 전달한다.
- AnimInstance가 읽을 수 있도록 "현재 상태 스냅샷"을 노출(읽기 전용 getter)한다.

### (권장) USandboxSkillComponent (Skill Execution Owner)
- SkillState의 Source of Truth.
- SkillState: Idle / Casting / Executing / Cooldown 를 보관한다.
- 상태 전이는 Tick 없이 Timer/Delegate로만 수행한다.
- 스킬 시작 시 몽타주 재생을 요청하고, OnMontageEnded/BlendingOut delegate를 바인딩해 종료를 감지한다.
- Cancel/Interrupt 요청을 받으면 CanCancel(Reason)을 통해 수락/거절을 결정한다.
- 전이가 발생하면 항상 ExitCurrentState() -> EnterNextState() 순서로 수행해 정리를 보장한다.
- ControlStateComponent에 "조작 제한 요청(Reason)"을 Add/Remove 한다 (직접 Set/Unset 하지 않는다).

### (권장) USandboxControlStateComponent (Control Authority)
- 조작 제한의 최종 판정(Authority).
- 상위 OverrideState(강제): None / Stunned / Cinematic / Dead 등 단일 값(덮어쓰기)
- 하위 RequestReasons(합성): MoveLock, InputLock, LookLock 등 "제한 사유" 집합
- 최종 결과(FinalControlSnapshot):
  - CanMove, CanInput, CanLook 등을 계산해 제공한다.
- 계산 결과에 따라 실제 실행을 적용한다:
  - 이동 입력 차단: Controller->SetIgnoreMoveInput(true/false)
  - 필요 시 즉시 정지: CharacterMovement->StopMovementImmediately()
  - (선택) 시야 입력 차단: Controller->SetIgnoreLookInput(true/false)
- AnimInstance는 이 컴포넌트의 FinalControlSnapshot을 읽기만 한다.

### UAnimInstance (표현 계층)
- SkillState/FinalControlSnapshot을 읽어서 애니메이션 블렌딩/슬롯만 결정한다.
- "이동을 막는다", "상태를 전이한다", "잠금 해제를 한다" 같은 게임플레이 결정을 하지 않는다.
- Notify는 이펙트/판정/타이밍 이벤트 용도로만 사용하고, 잠금 on/off 같은 권한 작업은 하지 않는다.

## Data Flow

### 1) 스킬 시작
1. Enhanced Input -> PlayerController가 스킬 입력 이벤트를 받는다.
2. Character에게 "스킬 사용 요청" 전달.
3. Character는 SkillComponent의 TryStartSkill(SkillId) 호출.
4. SkillComponent:
   - 현재 SkillState가 Idle인지 확인
   - 가능하면 Enter(Casting)로 전이
   - Casting Enter:
     - Casting Timer 시작 (예: 0.3s)
     - ControlStateComponent에 MoveLock/InputLock Reason 추가(스킬 정책에 따라)

### 2) Casting -> Executing
5. Casting Timer 만료 시 SkillComponent가 Transition(Executing)
6. Executing Enter:
   - 몽타주 재생 요청
   - OnMontageEnded 또는 OnMontageBlendingOutStarted delegate 바인딩
   - (필요 시) MoveLock 유지, 또는 InputLock만 유지 등 정책 적용
   - Notify는 "히트 발생" 같은 이벤트만 트리거

### 3) Executing 종료 -> Cooldown
7. 몽타주 종료(Ended/BlendingOut) 콜백 수신
8. SkillComponent가 Transition(Cooldown)
9. Cooldown Enter:
   - Cooldown Timer 시작 (예: 1.5s)
   - 입력/이동 제한은 보통 해제(단, 스킬 정책에 따라 일부 유지 가능)

### 4) Cooldown -> Idle
10. Cooldown Timer 만료 시 Transition(Idle)
11. Idle Enter:
   - 스킬 관련 모든 Reason handle 해제 보장

### 5) Cancel / Interrupt (정상 종료와 동일한 Exit 경로)
- 외부 이벤트(회피 입력/피격/CC)가 SkillComponent에 RequestCancel(Reason)을 보낸다.
- SkillComponent는 CanCancel(Reason, CurrentSkillState)를 평가해 수락/거절한다.
- 수락 시:
  - Transition(Idle 또는 Cooldown 정책) 수행
  - Exit에서 몽타주 델리게이트 언바인딩 + 스폰 리소스 정리 + Reason handle 해제 수행
- 핵심: 어떤 이유로 종료되든 Exit로 수렴해서 정리가 보장된다.

## Edge Cases

1) 몽타주가 중간 Stop되어 Notify가 호출되지 않는 경우
- Notify는 잠금 해제에 사용하지 않는다.
- 종료는 OnMontageEnded/BlendingOut delegate로 보장한다.

2) Cancel/Interrupt가 Casting 중 발생
- Casting Timer를 반드시 정리하고, Reason handle 해제 후 Idle/Cooldown으로 전이한다.

3) 동일 프레임에 Cancel과 MontageEnded가 같이 들어오는 경우
- SkillComponent는 "현재 상태/현재 실행 토큰"을 기준으로 한 번만 전이하도록 가드한다.
  (예: CurrentExecutionId로 콜백 필터링)

4) MoveLock 해제 누락/중복 해제 문제
- bool이 아니라 handle 기반 Add/Remove를 사용한다.
- SkillComponent는 자신이 Add한 handle만 보관/해제한다.

5) 이동이 멈춰야 하는 순간(채널링 시작 시) 캐릭터가 관성으로 미끄러지는 문제
- MoveLock 적용 시점에 StopMovementImmediately()를 1회 호출한다.
- 이후는 IgnoreMoveInput으로 입력만 차단한다.

## Perf / GC / Tick Notes
- Tick 사용 금지. 상태 전이는 Timer/Delegate로만 수행.
- State 구현은 우선 UENUM + SkillComponent 내 Enter/Exit 함수들로 구현 (가벼움, GC 부담 적음).
- Delegate 바인딩/언바인딩은 Executing Enter/Exit에서만 수행하여 누수 방지.
- Notify는 이벤트 트리거 용도로만 사용하고, 상태/잠금 결정 로직을 넣지 않는다.

## Replication Notes (선택)
- 확장 시 Authority는 서버:
  - 클라는 TryStartSkill/RequestCancel 요청만 보내고,
  - 서버가 SkillState 전이를 확정한다.
- SkillState와 ControlSnapshot은 필요 시 Replicate(또는 서버 이벤트로 동기화) 가능하도록 설계.