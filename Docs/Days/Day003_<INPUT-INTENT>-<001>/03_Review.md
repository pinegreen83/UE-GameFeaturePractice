## What Worked
- InputAction을 “키와 분리된 트리거”로 다루는 개념이 명확해졌다.
- 콘솔 커맨드(Exec) 기반 테스트 하네스를 통해 UI 없이도 입력/설정 흐름을 빠르게 검증할 수 있었다.
- IMC 적용 위치를 PlayerController로 단일화하면서 “입력 구성 책임”을 분리하는 방향이 잡혔다.

## What Broke
- UE5.4 환경에서 UInputMappingContext::GetMappings()가 const로만 제공되는 케이스가 있어,
    - IMC 매핑을 런타임에 직접 수정하는 접근이 막혔다.
- PlayerMappable 기반 리바인드도 즉시 적용이 어려웠고, 이 과정에서 티켓 목표 대비 과도한 시간이 소모됐다.
- “OK 로그”와 “실제 동작”의 괴리(덤프 기준 변경 없음)가 발생하면서 디버깅 비용이 커졌다.

## Fix / Improvement
- 리바인드 구현을 “완성”하지 않고 학습/경계 확인으로 정리하고 중단했다.
- 리바인드 대신, 본래 목적이었던 Input → Intent → Resolver 구조로 초점을 되돌리는 것이 더 생산적이라는 결론을 내렸다.
- 다음 재도전 시에는:
    - (1) PlayerMappable이 정상 동작하는 샘플/레퍼런스 프로젝트를 먼저 확보
    - (2) 해당 흐름을 그대로 이식 순서로 접근한다.

## Next Step
- 리바인드 구현은 보류하고, 아래를 다음 티켓으로 진행:
	1.	FInputIntent 설계(Pressed/Released/Hold, WorldDir/Aim, TimeStamp, Owner 등)
	2.	Intent 생성 위치 확정(PC/Pawn/Component 중 “한 곳”)
	3.	Resolver가 ECommandId를 반환하는 구조로 연결
	4.	Executor에서 CommandId 기반으로 Skill/State/Movement 호출(브로드캐스트 금지, 단일 실행 지점 유지)