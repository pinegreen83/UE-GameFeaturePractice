[Task ID]

PLAYER-STATE-LOCK-001

[Title]

플레이어 입력 가능 상태를 중앙에서 제어한다 (Input Lock / Unlock)

[Background]

지금까지는
- 스킬 실행 중
- 피격
- 구르기 / 경직
같은 상황에서 “입력이 되냐 안 되냐”가
각 기능 내부에서 암묵적으로 처리되고 있을 가능성이 높다.

실무에서는 이게 제일 먼저 터진다:
- 어떤 상태에선 공격이 나가고
- 어떤 상태에선 이동만 막히고
- 나중에 CC 하나 추가하면 전부 깨짐

[Requirements]
- 플레이어의 “입력 가능 상태”를 단일 지점에서 관리한다
- 최소 상태:
  - Free (모든 입력 허용)
  - Locked (모든 입력 차단)
  - Limited (이동 가능 / 스킬 불가 등 부분 허용)
- 스킬/피격/회피 등은
  “입력을 직접 막지 않고” 상태 요청만 보낸다
- 실패 케이스 1개:
  - 이미 Locked 상태일 때 다시 Lock 요청이 와도 안전해야 함

[Constraints]
- UE 5.4
- Tick 금지
- PlayerController / Character 책임 명확히 분리
- 네트워크 고려 (Authority 기준 상태 확정)

[Definition of Done]
- 로그로 현재 입력 상태가 항상 설명 가능하다
- 스킬 코드 안에 EnableInput / DisableInput 호출이 없다
- 이후 CC / Stun / Silence 추가 시 구조를 안 깨도 된다