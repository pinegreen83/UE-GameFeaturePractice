## Implementation Summary
본 티켓에서는 멀티플레이 환경에서 월드 오브젝트 상호작용의 권한과 책임을 명확히 분리하는 구현을 진행했다.
플레이어는 Overlap을 통해 상호작용 후보를 인지하고 UI를 표시하지만, 실제 월드 상태 변화(문 열림, 픽업 등)는 반드시 서버 승인 이후에만 발생하도록 구조를 고정했다.
자동 처리 오브젝트(힐킷)는 서버에서만 Overlap을 기준으로 처리하여, 경쟁 상황에서도 단일 결과로 수렴함을 검증했다.

## Files
- Interaction/Interactable.h
- Interaction/InteractionComponent_WO_IN_001.h / .cpp
- Actors/Pickup_HealKit.h / .cpp
- SandboxCharacter.h / .cpp (Interaction Sphere 및 연결)

## Core Logic
1) 의도 생성과 서버 요청 분리
```
// Client
void UInteractionComponent_WO_IN_001::AttemptInteract()
{
	if (FocusActor.IsValid())
	{
		Server_RequestInteract(FocusActor.Get());
	}
}
```
```
// Server
void UInteractionComponent_WO_IN_001::Server_RequestInteract_Implementation(AActor* Target)
{
	if (ValidateInteract_Server(Target))
	{
		IInteractable::Execute_ExecuteInteract(Target, GetOwner());
	}
}
```

2) 자동 픽업은 서버 Overlap 전용
```
void APickup_HealKit::OnSphereBeginOverlap(...)
{
	if (!HasAuthority() || bConsumed)
	{
		return;
	}

	ConsumeBy(OtherActor);
	Destroy();
}
```

3) 클라이언트 UI 전용 Overlap
```
void ASandboxCharacter::OnInteractSphereBeginOverlap(...)
{
	if (InteractComponent)
	{
		InteractComponent->NotifyBeginOverlap(OtherActor);
	}
}
```

## Design Match Check
- 설계 일치
    - 월드 상태 변화는 서버에서만 발생
    - 클라이언트 Overlap은 UI/후보 인지 용도로만 사용
    - 상호작용 요청은 플레이어(의도 주체)에서만 생성
- 의도적 단순화
    - 실패 사유 피드백, 예측 처리, 인터랙션 우선순위는 제외
    - 본 티켓에서는 “권한 흐름 검증”에만 집중

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 공통 Interaction 시스템으로 통합 예정.