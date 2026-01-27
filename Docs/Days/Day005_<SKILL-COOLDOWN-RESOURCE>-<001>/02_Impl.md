## Implementation Summary
- 스킬 요청/승인 흐름을 서버 권한 단일 결정으로 고정했다.
- 스킬 사용 진입점을 RequestActivateSkill()로 통일하고,
    - 클라는 Server_TryActivateSkill()로 요청만
    - 서버는 TryActivateSkill_InternalServer()에서 Resolve → Validate → Commit → Execute 수행
- 결과 전파는 Server RPC 진입점 한 곳에서만
    - 성공: Multicast_OnSkillResult
    - 실패: Client_OnSkillResult (요청자에게만)
- 쿨타임/자원 검증을 Validate에서 수행하고, Commit에서만 상태 변경(스태미나 차감/쿨타임 갱신)했다.
- 멀티 인스턴스(ListenServer + Clients)에서 로그 추적이 가능하도록 [NetMode][Owner][LocalRole/RemoteRole][PID] 태그를 추가했다.

## Files
- SK_SkillComponent.h/.cpp
    - 네트워크 진입점/내부 로직 분리
    - Resolve/Validate/Commit/Execute 구현
    - 결과 전파(성공 Multicast / 실패 Client)
    - 로그 태그 헬퍼 추가
- SK_SkillTypes.h
    - FSK_SkillIntent, FSK_SkillValidationResult, FSK_SkillResult
    - ESK_SkillFailReason 및 Reason -> string 변환 함수(SK_ToString)
    - FSK_SkillRuntimeState(CooldownEndTime 기반)
- (프로젝트 상황에 따라) SK_StatComponent.h/.cpp 또는 기존 StatComponent 연동
    - GetStamina(), ConsumeStamina() 등 자원 API 사용

## Core Logic
1) 네트워크 진입점에서만 결과 전파
```
void USK_SkillComponent::Server_TryActivateSkill_Implementation(FName SkillId, const FSK_SkillIntent& Intent)
{
	FSK_SkillResult Result;
	const bool bApproved = TryActivateSkill_InternalServer(SkillId, Intent, Result);

	if (bApproved)
	{
		Multicast_OnSkillResult(SkillId, Result, Intent);
	}
	else
	{
		Client_OnSkillResult(SkillId, Result);
	}
}
```

2) 내부 로직은 순수 처리( RPC 호출 금지 )
```
bool USK_SkillComponent::TryActivateSkill_InternalServer(FName SkillId, const FSK_SkillIntent& Intent, FSK_SkillResult& Out)
{
	FSK_SkillContext Ctx; FSK_SkillResult Fail;
	if (!ResolveContext(SkillId, Ctx, Fail)) { Out = Fail; return false; }

	const auto V = Validate(Ctx, Intent);
	if (!V.bCanActivate) { Out = {/* fail payload */}; return false; }

	Commit(Ctx, Intent);
	Execute(Ctx, Intent);

	Out.bSucceeded = true;
	return true;
}
```

3) Resolve/Validate는 읽기 전용, Commit만 쓰기
```
// Commit (서버만)
FSK_SkillRuntimeState& Runtime = RuntimeStateMap.FindOrAdd(Ctx.SkillDef->SkillId);
Runtime.CooldownEndTime = Now + Ctx.SkillDef->CooldownSeconds;
```

4) FailReason 가독성 + 인스턴스 구분 로그
- FailReason을 숫자 대신 문자열로 출력 (SK_ToString)
- [ListenServer/Client][Owner][LRole/RRole][PID] 태그를 로그에 포함

## Design Match Check
- ✅ 단일 진입점 유지: 입력은 RequestActivateSkill()만 호출
- ✅ 책임 분리: Resolve/Validate(읽기) vs Commit/Execute(쓰기) 분리
- ✅ 서버 권한: Commit/Execute는 서버에서만 발생(로그로 증명)
- ✅ 실패 이유 분리: Cooldown/Resource/State 등을 Reason으로 구분
- ✅ 전파 규칙 고정: 성공=Multicast / 실패=Client(요청자만)
- ✅ Tick 미사용: 시간 비교(CooldownEndTime) 기반
- (선택) SkillDef 캐시/Resource 분리 구조는 v1 방향성 유지(무거운 리소스는 하드레퍼런스 금지)

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.