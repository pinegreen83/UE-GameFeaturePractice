[Task ID]

UI-FEEDBACK-BASE-001

[Title]

스킬 실행 결과를 UI 피드백으로 명확히 전달한다

[Background]

지금 구조는 스킬 실행의 성공/실패는 내부 로직과 로그로만 확인 가능해.
실무에서는 디자이너·QA·플레이어 모두가 “왜 안 됐는지”를 즉시 인지해야 하고,
이 책임은 스킬이나 캐릭터가 아니라 UI 피드백 레이어에 있어야 한다.

[Requirements]
- 스킬 실행 결과를 UI에 표시한다
- 성공 / 실패 구분
- 실패 시 사유 표시 (쿨다운, 자원 부족, 상태 락 등 최소 1개)
- UI는 Skill 시스템을 직접 참조하지 않는다
- 동일 프레임에 중복 출력되지 않도록 한다

Failure Case
- 서버 승인 실패 시, 클라이언트에서 성공 UI가 먼저 뜨면 안 된다

[Constraints]
- UE 5.4 기준
- Skill → UI 직접 호출 금지
- Delegate 또는 Interface 기반 전달
- Tick 사용 금지
- 네트워크 환경 (Listen Server) 고려

[Definition of Done]
- 스킬 실행 성공/실패가 UI에서 즉시 구분 가능
- 실패 사유가 로그 없이도 확인 가능
- UI 제거 시 Skill 시스템 수정이 필요 없다
- 서버 승인 결과 기준으로 UI가 출력된다