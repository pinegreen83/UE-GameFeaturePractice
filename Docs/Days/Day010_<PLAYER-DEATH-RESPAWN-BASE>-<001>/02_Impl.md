## Implementation Summary
이번 티켓에서는 플레이어 사망(Death)을 서버 단일 결정으로 처리하고,
상태 변화는 델리게이트 기반 이벤트 흐름으로 전파되도록 구현했다.

HP 감소는 StatComponent가 담당하며,
HP가 0이 되었을 때 사망 여부 판단은 LifeStateComponent가 서버에서 단일하게 결정한다.
사망 상태 전환 이후의 입력 차단, 연출, UI 반응은
각 시스템이 OnLifeStateChanged 델리게이트를 구독하여 반응하는 구조로 구현하였다.

## Files
- LifeStateTypes_PL_DE_001.h
- StatComponent_PL_DE_001.h / .cpp
- LifeStateComponent_PL_DE_001.h / .cpp
- SBPlayerController_PL_DE_001.h / .cpp
- SandboxCharacter.h / .cpp (컴포넌트 추가 및 기본 구조 유지)

## Core Logic
서버 권한 기반 상태 전환
- HP 변경은 서버 커밋 단계에서만 수행
- HP ≤ 0 → StatComponent가 사실 이벤트(OnHealthDepleted) 발행
- LifeStateComponent가 이를 구독하여
    - 서버에서만 LifeState = Dead로 전환
    - RepNotify + 델리게이트로 전파

상태 변화 전파 방식
```
StatComponent (HP 변경)
 → OnHealthDepleted
   → LifeStateComponent (서버에서 Dead 결정)
     → LifeState Replication
       → OnLifeStateChanged Delegate
         → Controller / Character / UI 반응
```

입력 차단
- PlayerController가 LifeStateComponent의 델리게이트를 구독
- Dead 진입 시:
    - SetIgnoreMoveInput(true)
    - SetIgnoreLookInput(true)
- Alive 복귀 시 입력 복구

## Design Match Check
- ✅ 상태 결정은 단일 컴포넌트(LifeStateComponent)에서만 수행
- ✅ StatComponent는 수치 관리 및 “사실 통지” 역할만 담당
- ✅ 직접 함수 호출 체인 없이 이벤트/델리게이트 기반 구조 유지
- ⚠️ 리슨 서버 환경에서 컴포넌트 누락 시 이벤트가 조용히 실패하는 케이스- 

## Commit
- 로컬 실험용 구현 (단일 티켓 연습)
- 사망/리스폰 시스템은 이후 전투/상태 확장 티켓에서 통합 예정