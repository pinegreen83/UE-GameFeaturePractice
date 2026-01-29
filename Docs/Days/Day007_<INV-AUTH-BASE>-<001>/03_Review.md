## What Worked
- 서버 Authority 단일 진실 구조가 명확하게 동작함
- PlayerId 기반 로그를 통해 요청자 / 처리 대상 / 복제 수신자를 명확히 구분 가능
- 슬롯 기반 인벤토리 로직(Add / Remove / MaxStack)이 안정적으로 동작
- Replication 타이밍(요청 직후 vs OnRep 수신)을 명확히 이해함

## What Broke
- 초기에는 Exec 커맨드를 Character에 두어

    멀티 환경에서 다른 캐릭터의 인벤토리가 수정되는 문제 발생
- 클라이언트 로그 시점과 복제 결과를 혼동하여

    “값이 안 바뀐 것처럼 보이는” 착시 발생
- Remove 로직에서 수량 부족 시 부분 제거가 발생할 수 있는 구조 발견

## Fix / Improvement
- 디버그 입력 진입점을 PlayerController로 이동하여
    
    요청 대상 Pawn을 확정
- 로그에 PlayerId / LocalRole / Authority 정보를 포함시켜
    
    네트워크 흐름을 명확히 추적
- Remove 로직을 “All-or-Nothing” 정책으로 수정하여
    
    수량 부족 시 상태 변화 없도록 개선

## Next Step
- 월드 아이템 Actor 기반 픽업 시스템 구현
    - 서버 검증
    - 동시 습득 경쟁 처리
- 인벤토리 슬롯 UI(그리드 + 슬롯 위젯)
- 저장/로드(DB 또는 SaveGame)
- 퀘스트/업적 시스템과 인벤토리 이벤트 연동