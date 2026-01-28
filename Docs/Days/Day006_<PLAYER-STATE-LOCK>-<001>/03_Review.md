## What Worked
- 입력 → 정책판정 → 라우팅이 한 줄 흐름으로 보이면서 디버깅이 쉬워짐.
- 공통 상태 체크가 분산되지 않고 Gate로 모임(콜백마다 if 덕지덕지 방지).
- ToggleStunSelf가 Host/Client 각각 독립적으로 작동하여 네트워크 검증 루프가 빠름.
- “서버는 클라를 신뢰하지 않는다” 원칙을 서버 최종 판정으로 코드에 고정함.

## What Broke
- Host/Client 창을 오가며 테스트할 때,
    - Gate 로그(거부)와 서버 로그(accept)가 섞여 보여서
    - “왜 reject가 안 뜨지?”처럼 오해가 생김.
- 로그에 Pawn 식별 정보가 없으면 (C_0 / C_1) 구분이 어려움.

## Fix / Improvement
- 로그에 GetNameSafe(GetPawn()) / GetName() 등을 포함해서
    - “누가(어느 Pawn)”의 이벤트인지 확실히 구분하도록 개선 권장.
- (구조 안전성) Gate와 서버 판정이 참조하는 상태가 항상 동일하도록,
    - StatusComponent 접근 경로를 통일하거나(멤버 vs FindComponent 혼용 주의)
    - BP에서 컴포넌트 중복 부착 같은 실수를 방지하는 가이드가 필요.

## Next Step
- Rejected 피드백을 HUD/Widget에 붙여서 “왜 막혔는지”를 즉시 보이게 하기.
- 입력 이벤트 타입 확장(예: Hold/Cancel)로 차지/채널링 입력까지 확장.
- Gate에서 공통 정책을 Stun 외에도 확장 가능한 형태(예: bitmask / gameplay tag / reason code)로 발전.