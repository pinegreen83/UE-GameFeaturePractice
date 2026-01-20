## What Worked
- 의도(Intent) / 실행(Context) / 확정(Authority) 구조가 실제 네트워크 환경에서 명확히 분리됨
- 로그 기반 검증으로 “서버에서만 데미지가 적용됨”을 명확히 증명
- Controller / Character / Component 역할 경계가 코드로 자연스럽게 유지됨
- TMap 복제 불가 문제를 구조 변경으로 해결하며 UE 실무 제약 학습

## What Broke
- TPair 사용 시 UPROPERTY 리플렉션 불가 이슈
- TMap 직접 Replication 불가로 쿨다운 구조 수정 필요
- 초기 쿨다운 값으로 인한 서버 Validate 실패(의도된 동작이었으나 혼동 발생)

## Fix / Improvement
- TPair → 명시적 USTRUCT(FSkillStat)로 교체
- 쿨다운 데이터:
    - 서버/클라 캐시용 TMap
    - 복제용 TArray<FCooldownEntry>
- 쿨다운 EndTime 기준 서버 확정 방식 유지
- ApplyDamage 직전 로그 추가로 서버 전용 실행 증명

## Next Step
- 서버에서 AimDir / Origin 재구성(치트 방지 강화)
- 클라이언트 예측 실패 시 UI 롤백 처리
- 쿨다운 만료 엔트리 정리 로직 추가
- 동일 구조를 인벤토리, 장비 교체, UI 요청 등에 확장 적용