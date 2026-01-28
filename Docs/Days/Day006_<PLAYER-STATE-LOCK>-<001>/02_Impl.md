## Implementation Summary
- PlayerController가 Enhanced Input 바인딩을 전담하고, 콜백은 전부 InputGateComponent로만 전달.
- InputGateComponent(PC 소유)가 공통 정책(이번 티켓: Stun)을 한 곳에서 검사하고, 통과 시 Pawn의 타입 기반 Interface로 전달.
- StatusComponent(캐릭터 소유, 서버 권위 + Replicate)가 Stun 상태를 서버에서 확정하고, 클라로 복제.
- Exec ToggleStunSelf는 실행한 플레이어의 Pawn(Self)만 토글되도록 구성(Host/Client 각각 독립 테스트 가능).
- 서버는 클라를 신뢰하지 않으므로, 공격 입력은 서버에서 최종 판정(Stun이면 reject)하도록 구성.

## Files
- InputConsumerInterface.h
- StatusComponent.h/.cpp
- InputGateComponent.h/.cpp
- SandboxCharacter_InputGate.h/.cpp
- SandboxPlayerController_InputGate.h/.cpp

## Core Logic
1) PC → Gate 라우팅(콜백은 얇게)
```
// PC 콜백: 로직 없음, Gate로만 전달
void ASandboxPlayerController_InputGate::OnAttackStarted(const FInputActionValue&)
{
    InputGate->RequestAttackPressed();
}
```

2) Gate 공통 정책 체크 후 Interface 호출
```
EInputRejectReason UInputGateComponent::CheckCommonPolicy() const
{
    APawn* Pawn = ControlledPawn.Get();
    if (!Pawn) return EInputRejectReason::MissingPawn;

    const UStatusComponent* Status = Pawn->FindComponentByClass<UStatusComponent>();
    if (!Status) return EInputRejectReason::MissingStatus;

    if (Status->IsStunned()) return EInputRejectReason::Stunned;

    return EInputRejectReason::None;
}

void UInputGateComponent::RequestAttackPressed()
{
    if (CheckCommonPolicy() != EInputRejectReason::None)
    {
        OnInputRejected.Broadcast(TEXT("AttackPressed"), EInputRejectReason::Stunned);
        return;
    }

    if (IInputConsumerInterface* Consumer = Cast<IInputConsumerInterface>(ControlledPawn.Get()))
    {
        Consumer->ConsumeTrigger_Attack_Pressed();
    }
}
```

3) 서버 최종 판정(클라 신뢰 금지)
```
void ASandboxCharacter_InputGate::ConsumeTrigger_Attack_Pressed()
{
    if (HasAuthority()) { DoAttackPressed_ServerAuth(); }
    else { Server_AttackPressed(); }
}

void ASandboxCharacter_InputGate::DoAttackPressed_ServerAuth()
{
    if (StatusComponent && StatusComponent->IsStunned())
    {
        UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackPressed rejected: Stunned"));
        return;
    }

    UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackPressed accepted"));
}
```

4) Exec로 Self만 Stun 토글 (Host/Client 각각 독립)
```
UFUNCTION(Exec)
void ToggleStunSelf()
{
    if (HasAuthority())
    {
        if (UStatusComponent* Status = GetPawn()->FindComponentByClass<UStatusComponent>())
        {
            Status->ToggleStunned_ServerAuth();
        }
        return;
    }

    Server_ToggleStunSelf(); // 클라 → 서버 요청
}
```

## Design Match Check
- ✅ 입력 책임은 PC: 모든 바인딩/콜백은 PC에 집중.
- ✅ 공통 정책은 Gate로 집중: 상태(Stun) 체크를 한 곳에서 수행.
- ✅ 상태는 캐릭터 소유 + 서버 권위: StatusComponent Replicate, 변경은 서버에서만.
- ✅ 서버 최종 판정: 공격 입력은 서버에서 다시 체크하여 “서버가 절대 진리” 유지.
- ⚠️ 테스트 혼선 포인트: Host/Client를 번갈아 눌렀을 때 로그만 보면 “Stun인데 0”처럼 보일 수 있어, 항상 “어느 Pawn의 로그인지” 함께 찍는 게 안전.

## Commit
- 로컬 실험용 구현 (단일 티켓 연습).
- 추후 통합 시 별도 커밋 예정.