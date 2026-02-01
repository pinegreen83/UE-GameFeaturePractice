## What Worked
- 스킬 승인 파이프라인(기존 Day004)을 크게 건드리지 않고도 결과 전달 구조를 확장할 수 있었음
- FeedbackEvent를 데이터 중심으로 설계하여 UI/로그와 명확히 분리됨
- Subsystem + Delegate 구조로 확장 포인트가 명확해짐

## What Broke
- 리슨 서버 환경에서 DebugComponent가 서버/클라이언트 양쪽 인스턴스에 바인딩되어 동일 로그가 2회 출력됨
- 컴포넌트 클래스 변경 이력으로 인해 BP 직렬화 충돌 발생

    → SkillComponent 참조가 nullptr로 초기화되는 문제 경험


## Fix / Improvement
- 중복 로그 원인이 “이벤트 중복 발행”이 아니라 동일 Subsystem에 여러 리스너가 바인딩된 구조적 문제임을 확인
- Debug Sink는 캐릭터가 아닌 PlayerController / HUD / LocalPlayer 단위로 두는 것이 더 적절하다는 결론
- 서브오브젝트 이름 충돌 및 BP 직렬화 문제에 대한 이해 확보

    → C++ 생성 vs BP 부착 중 하나로 통일 필요성 인식

## Next Step
- FeedbackDebugComponent를 로컬 전용 위치(PlayerController/HUD) 로 이동
- 실패 Stage를 검토/타당성/적용/Commit 단계별로 더 세분화
- FeedbackEvent를 스킬 외 시스템(인벤토리, 월드 인터랙션)에도 재사용
- Shared 레이어로 승격할 클래스 선별 및 통합 작업 진행