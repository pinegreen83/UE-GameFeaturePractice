## Implementation Summary
클라이언트는 스킬 사용 요청 시 입력 자체가 아닌 ‘의도(Intent)’만 서버에 전달한다.
서버는 해당 의도를 기준으로 현재 월드 상태, 캐릭터 상태, 자원(스태미나), 쿨타임을 검증한 뒤
승인 시에만 결과를 Multicast RPC로 전파하고, 거절 시에는 요청한 클라이언트에게만 Client RPC로 결과를 전달한다.

클라이언트는 결과에 따라 연출만 수행하며, 게임의 사실(State 변화)은 서버만이 확정한다.

## Files
- SandboxSkillTypes.h
    - 스킬 요청/결과에 사용되는 FSandboxSkillIntent, FSandboxSkillResult, ESandboxSkillRejectReason 정의
- SkillComponent_SK_NE_001.h / .cpp
    - 스킬 요청 처리의 핵심 컴포넌트
    - 서버 검증, 승인/거절 판정, RPC 전파 로직 구현
- SandboxCharacter.cpp
    - 테스트용 스킬 요청 진입점 제공 (Test_RequestSkill1)

## Core Logic
클라이언트 → 서버 (의도 전달)
```
void USkillComponent_SK_NE_001::RequestSkillUseSimple(FName SkillId)
{
	FSandboxSkillIntent Intent;
	Intent.SkillId = SkillId;
	Intent.ClientOrigin = GetOwner()->GetActorLocation();
	Intent.ClientForward = GetOwner()->GetActorForwardVector();

	Server_RequestSkillUse(Intent);
}
```

서버 검증 및 판정
```
FSandboxSkillResult USkillComponent_SK_NE_001::Server_ValidateAndBuildResult(
	const FSandboxSkillIntent& Intent) const
{
	if (bCooldownActive_Server)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason::Cooldown;
		return Result;
	}

	if (Stamina < SkillCost)
	{
		Result.bApproved = false;
		Result.RejectReason = ESandboxSkillRejectReason::NotEnoughStamina;
		return Result;
	}

	Result.bApproved = true;
	return Result;
}
```

승인 / 거절 전파
```
if (Result.bApproved)
{
	Server_ApplySkillImpact(Result);
	Multicast_PlaySkill(Result);
}
else
{
	Client_RejectSkill(Result);
}
```

## Design Match Check
- 의도 기반 요청 구조: 설계와 동일하게 입력 자체가 아닌 Intent 구조체 전달
- 서버 권한 확정: 스태미나 감소, 쿨타임 판정은 서버에서만 수행
- 전파 정책 분리
    - 승인: NetMulticast
    - 거절: Client RPC (Owner Only)
- 클라이언트 책임 최소화: 연출만 수행, 상태 변경 없음

→ 설계와 구현이 1:1로 일치함을 로그를 통해 검증 완료.

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.