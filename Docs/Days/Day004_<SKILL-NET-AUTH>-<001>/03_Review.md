## What Worked
- 의도(Intent) / 결과(Result) 분리로 네트워크 책임이 명확해짐
- 서버에서만 상태를 확정하도록 강제하여 신뢰 모델이 분명해짐
- NetMode / Role / Time을 로그에 포함시켜 Listen Server 환경에서도 흐름 추적 가능
- Cooldown / Stamina 등 거절 케이스가 로그로 명확히 드러남

## What Broke
- 커스텀 로그 카테고리 선언 후 DEFINE_LOG_CATEGORY 누락으로 링커 에러 발생
- Listen Server 환경에서 Client RPC 로그가 Authority로 보이는 역할 혼동 발생
- 초기 로그에 타임스탬프가 없어 연속 입력 시 이벤트 순서가 헷갈림

## Fix / Improvement
- 로그 카테고리 선언/정의 분리 원칙 정리 후 문제 해결
- 로그 출력 시 다음 정보 추가:
    - World TimeSeconds
    - NetMode (ListenServer / Client)
    - Actor Name
    - LocalRole (Authority / Autonomous / Simulated)
- 이를 통해 서버/클라 착시 문제를 로그 레벨에서 해소

## Next Step
- RequestId 또는 Sequence를 Result에 추가하여 승인/거절/Multicast를 하나의 흐름으로 추적
- 실제 스킬 영향(데미지 또는 HP 감소) 추가
- 메이플식 연출 예측(Visual Prediction) 실험
- Dedicated Server 환경에서 동일 구조 재검증