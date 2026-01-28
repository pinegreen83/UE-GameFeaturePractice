// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day006-PLAYER-STATE-LOCK-001/SBPlayerController_PL_ST_001.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/InputGateComponent_PL_ST_001.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/StatusComponent_PL_ST_001.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

DEFINE_LOG_CATEGORY_STATIC(LogInputGatePC, Log, All);

ASBPlayerController_PL_ST_001::ASBPlayerController_PL_ST_001()
{
	InputGateComponent = CreateDefaultSubobject<UInputGateComponent_PL_ST_001>(FName("InputComponent"));
}

void ASBPlayerController_PL_ST_001::BeginPlay()
{
	Super::BeginPlay();
	ApplyIMC();
	RefreshGatePawn();
}

void ASBPlayerController_PL_ST_001::OnPossess(APawn* inPawn)
{
	Super::OnPossess(inPawn);
	RefreshGatePawn();
}

void ASBPlayerController_PL_ST_001::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogInputGatePC, Warning, TEXT("EnhancedInputComponent not found."));
		return;
	}
	
	// Move: Triggered/Completed
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASBPlayerController_PL_ST_001::OnMoveTriggered);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &ASBPlayerController_PL_ST_001::OnMoveCompleted);
	}

	// Attack: Started/Completed (Pressed/Released로 사용)
	if(IA_Attack)
	{
		EIC->BindAction(IA_Attack, ETriggerEvent::Started, this, &ASBPlayerController_PL_ST_001::OnAttackStarted);
		EIC->BindAction(IA_Attack, ETriggerEvent::Completed, this, &ASBPlayerController_PL_ST_001::OnAttackCompleted);
	}
}

void ASBPlayerController_PL_ST_001::ApplyIMC()
{
	if (!IsLocalController()) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;
	
	if(UInputMappingContext* InputMappingContext = InGameIMC.LoadSynchronous())
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void ASBPlayerController_PL_ST_001::RefreshGatePawn()
{
	if (InputGateComponent)
	{
		InputGateComponent->SetControlledPawn(GetPawn());
	}
}

void ASBPlayerController_PL_ST_001::ToggleStunSelf()
{
	// 리슨 서버(Host viewport)에서 실행하면 서버 로컬 플레이어만 토글
	// 클라에서 실행하면 서버 RPC로 요청(서버가 요청자 Pawn만 토글)
	if (HasAuthority())
	{
		APawn* Pawn = GetPawn();
		if(!Pawn) return;

		if (UStatusComponent_PL_ST_001* StatusComponent = Pawn->FindComponentByClass<UStatusComponent_PL_ST_001>())
		{
			StatusComponent->ToggleStun_ServerAuth();
			UE_LOG(LogInputGatePC, Warning, TEXT("[Server] ToggleStunSelf executed for %s"), *Pawn->GetName());
		}
		return;
	}

	Server_ToggleStunSelf();
}

void ASBPlayerController_PL_ST_001::Server_ToggleStunSelf_Implementation()
{
	APawn* Pawn = GetPawn();
	if(!Pawn) return;

	// 보안: 요청자 자신의 Pawn만
	if(UStatusComponent_PL_ST_001* StatusComponent = Pawn->FindComponentByClass<UStatusComponent_PL_ST_001>())
	{
		StatusComponent->ToggleStun_ServerAuth();
		UE_LOG(LogInputGatePC, Warning, TEXT("[Server] ToggleStunSelf (RPC) executed for %s"), *Pawn->GetName());
	}
}

void ASBPlayerController_PL_ST_001::OnMoveTriggered(const FInputActionValue& Value)
{
	if (!InputGateComponent) return;
	const FVector2D Axis = Value.Get<FVector2D>();
	InputGateComponent->RequestMoveAxis2D(Axis);
}

void ASBPlayerController_PL_ST_001::OnMoveCompleted(const FInputActionValue& Value)
{
	if (!InputGateComponent) return;
	InputGateComponent->RequestMoveAxis2D(FVector2D::ZeroVector);
}

void ASBPlayerController_PL_ST_001::OnAttackStarted(const FInputActionValue& Value)
{
	if (!InputGateComponent) return;
	InputGateComponent->RequestAttackPressed();
}

void ASBPlayerController_PL_ST_001::OnAttackCompleted(const FInputActionValue& Value)
{
	if (!InputGateComponent) return;
	InputGateComponent->RequestAttackReleased();
}
