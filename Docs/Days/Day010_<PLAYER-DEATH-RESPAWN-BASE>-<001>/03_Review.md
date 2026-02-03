## What Worked
- 사망 로직을 서버 단일 결정 구조로 명확히 분리함
- 델리게이트 기반 설계로
- 입력 차단
- 캐릭터 반응
- UI 처리를 서로 직접 참조 없이 연결 가능
- Tick 없이 이벤트 기반으로 FSM을 구성하여 구조가 단순해짐

## What Broke
- 클라이언트에서 사망 상태인데도 이동이 계속 가능한 문제 발생
- 로그상 서버에서는 Dead 전환이 정상적으로 수행되었으나, 클라이언트에서는 HandleLifeStateChanged가 호출되지 않음

## Fix / Improvement
- 원인: 클라이언트 Pawn에 LifeStateComponent가 누락
- 결과적으로:
    - RepNotify 대상 자체가 존재하지 않았고
    - 델리게이트 바인딩도 이루어지지 않았음
- 해결:
    - 캐릭터 BP에 LifeStateComponent 명시적으로 추가
    - 컴포넌트 존재 여부 로그를 통해 빠르게 원인 특정

이 과정에서 **이벤트 기반 구조의 주요 함정(조용히 실패하는 케이스)**를 명확히 체감함.

## Next Step
- LifeState 확장 (Stun / Knockback / Invincible 등)
- SkillComponent가 LifeState를 구독하여
    - Dead 시 스킬 자동 취소
    - 상태별 사용 제한 처리
- Respawn 시 초기화 순서 정리 (Stat → State → Input)