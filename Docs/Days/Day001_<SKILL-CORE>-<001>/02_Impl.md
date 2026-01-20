## Implementation Summary
본 티켓에서는 스킬 실행 구조의 책임 분리와 서버 권위 검증을 목표로,
스킬 데이터를 정적 정의(Static), 실행 의도(Intent), 실행 컨텍스트(Context),
그리고 서버 확정 로직으로 분리하여 구현하였다.

Controller / Character는 입력을 수집하여 Intent만 생성하고,
실제 스킬 실행의 책임은 USkillComponent가 가지며,
데미지 판정 및 쿨다운 확정은 서버에서만 수행되도록 구성하였다.

Listen Server 및 Client 환경에서 로그 기반 검증을 통해
- 클라이언트 예측 요청
- 서버 권위 실행
- 서버 전용 데미지 적용
이 정상적으로 분리되어 동작함을 확인하였다.

## Files
- SkillTypes_SKILL_CORE_001.h
    - 스킬 정적 데이터 구조체
    - Intent / Execution Context 정의
- SkillComponent_SKILL_CORE_001.h / .cpp
    - 스킬 실행 책임 컴포넌트
    - 서버 RPC, 쿨다운 관리, HitScan 실행
- ASandboxCharacter (기존 클래스 수정)
    - 입력 바인딩 및 Intent 생성

## Core Logic
1. 입력 -> Intent 생성
``` 
Intent.Origin = GetActorLocation();
Intent.AimDir = GetActorForwardVector();
SkillComponent->RequestActivateSkill("Test1", Intent);
```

2. Client / Server 공통 엔트리
```
bool USkillComponent::RequestActivateSkill(...)
```
- Client : 예측 연출 허용
- Server : 검증 요청 처리

3. Server 권위 실행
```
void ServerTryActivateSkill(...)
```
- 쿨 다운 검사
- 서버 컨텍스트 확정
- HitScan + ApplyDamage

4. 서버 전용 데미지 적용
```
UGameplayStatics::ApplyDamage(...)
```
- Client에서는 호출되지 않음 (로그로 검증)


## Design Match Check
|설계 항목|구현 결과|
|-------|-------|
|Static / Runtime 데이터 분리 | 구조체 분리 완료|
|Intent / Context 분리 | USTRUCT로 명확히 분리|
|Controller 책임 최소화 | 입력 + Intent 생성만 담당|
|실행 책임 Component 집중 | SkillComponent 중심|
|서버 권위 데미지 | 서버에서만 ApplyDamage|
|네트워크 검증 | Listen Server 로그로 증명|

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.