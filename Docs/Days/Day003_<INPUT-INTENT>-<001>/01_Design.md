## Goal
이번 티켓의 목표는 입력(키/마우스)과 게임플레이 의미(행동/스킬)를 분리하기 위한 기반을 만든다. 
구체적으로는 Enhanced Input에서 InputAction을 단순한 트리거로 유지하고, 
런타임에서 “입력 매핑을 변경(리바인드)”하는 실험을 통해 입력 레이어의 제약/한계를 확인한다. 
최종적으로는 리바인드 구현에 집착하지 않고, Input → Intent(티켓) → Resolver → Executor로 이어지는 확장 가능한 구조를 위한 발판을 마련한다.

## Class / Component Responsibilities	
- ASBPlayerController_SK_IN_001
    - 로컬 플레이어 입력 구성(매핑 컨텍스트 적용)
    - 콘솔(Exec) 커맨드로 리바인드/덤프 테스트 수행
    - 입력 이벤트 바인딩(테스트용)
- UInputRebindComponent_SK_IN_001
    - (초기 목표) 특정 InputAction의 키를 런타임에 교체
    - (현 상황) UE5.4 환경에서 IMC 매핑 직접 수정 및 PlayerMappable 경로가 막혀, “가능/불가능” 경계를 확인하는 역할
- UInputMappingContext / UInputAction (Assets)
    - InputAction은 “행동 축” 단위로 유지(Attack/Dodge/Interact 등)
    - 키는 MappingContext 레벨에서만 결정된다는 원칙을 유지

## Data Flow
1.	게임 시작 시 PlayerController::BeginPlay
- LocalPlayer Subsystem에 DefaultIMC를 추가(AddMappingContext)
- RebindComponent에 TargetIMC = DefaultIMC 지정
- 저장된 설정을 적용 시도(LoadAndApply)

2.	입력 바인딩 (SetupInputComponent)
- IA_Attack에 대한 바인딩을 걸고, 트리거 시 로그로 확인

3.	리바인드 시도 (콘솔 커맨드)
- SM_RebindAttackKey <KeyName> 실행
- RebindComponent->RebindActionKey(IA_Attack, NewKey) 호출
- 성공 여부를 로그로 출력

4.	덤프(검증)
- SM_DumpBindings로 현재 IMC가 가진 Action-Key 목록을 출력

## Edge Cases
- IMC 수정 가능 여부: UE5.4에서 UInputMappingContext::GetMappings()가 const로만 노출될 수 있어, 런타임 직접 수정이 불가할 수 있음.
- 중복 매핑: 같은 InputAction이 키보드/패드 등 여러 Key에 매핑되어 있을 수 있으며, “첫 매핑만 변경”하면 기대와 다르게 동작할 수 있음.
- 포커스/입력 라우팅: PIE 환경에서 입력 포커스/로컬 컨트롤러 여부에 따라 이벤트가 누락되는 것처럼 보일 수 있음.
- Asset 불일치: IA_Attack/DefaultIMC가 서로 다른 에셋을 참조하면, 매핑 교체 조건이 성립하지 않음.

## Perf / GC / Tick Notes
- Tick 사용 없음(이벤트 기반)
- 리바인드/덤프는 개발용 콘솔 커맨드 중심
- 컴포넌트는 PlayerController에 소유되어 GC 안전(UPROPERTY 유지 전제)

## Replication Notes (선택)
- 본 티켓의 입력/리바인드는 로컬 플레이어 설정 범주로 서버 복제 대상이 아님.
- 향후 멀티플레이 확장 시:
    - “입력” 자체가 아니라 Intent/Command를 서버로 전송(검증/확정)하는 구조가 더 적합.