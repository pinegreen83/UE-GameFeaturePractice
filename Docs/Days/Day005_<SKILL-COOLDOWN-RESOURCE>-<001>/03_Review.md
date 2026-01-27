## What Worked
- 서버에서만 Execute/Commit 되도록 강제되어, 멀티에서도 기준이 흔들리지 않았다.
- 실패 결과를 FailReason으로 분리하니 “왜 안 됐는지”가 명확해졌고, 디버깅 속도가 빨라졌다.
- NetMulticast가 인스턴스마다 1회 호출된다는 점을 로그 태그로 증명하면서, 네트워크 흐름을 “눈으로” 확인할 수 있었다.
- Resolve/Validate const 제약이 런타임 상태 수정 실수를 컴파일 타임에 막아줬다(설계가 코드로 강제됨).

## What Broke
- UPROPERTY()로 USTRUCT* 포인터를 노출해서 UHT 에러 발생
    
    → UPROPERTY는 GC/리플렉션 대상에만, Context는 비노출로 전환.
- TMap::Find()가 const 컨텍스트에서 const* 반환 → RuntimeState*에 못 담는 컴파일 에러
    
    → Resolve/Validate는 읽기 전용이어야 하므로 Context를 const FSkillRuntimeState*로 수정.
- FailReason을 숫자로 찍으니 로그 가독성이 떨어짐
    
    → enum → string 변환 함수 추가.

## Fix / Improvement
- 결과 전파를 내부 로직에서 직접 호출하지 않고, Server RPC 핸들러 한 곳으로 집약
    - 내부 로직은 “결과 생성”까지만
    - 서버 진입점에서 성공/실패에 따라 Multicast/Client RPC 선택
- 로그 태그를 표준화([NetMode][Owner][Role][PID])해서
    - “누가 눌렀는지 / 어디서 실행됐는지 / 왜 실패했는지”를 한 줄로 파악 가능하게 개선

## Next Step
- 실패 결과에 “요청자 식별자(Instigator PID)”를 Result에 포함해서, 멀티캐스트 로그만 봐도 “누가 트리거했는지”를 더 즉시 파악하도록 개선
- 스킬 정의 캐시/리소스 분리(SoftObjectPtr/ResourceId) 정책 확정 및 프리로드 전략(필요 시)
- Validate 항목 확장(거리/타겟/상태 태그) 시에도 Resolve/Validate/Commit 경계 유지하면서 Validator 모듈화(티켓 분리 추천)