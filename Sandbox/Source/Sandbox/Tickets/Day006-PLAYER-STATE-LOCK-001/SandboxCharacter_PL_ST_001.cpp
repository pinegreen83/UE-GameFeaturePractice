// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day006-PLAYER-STATE-LOCK-001/SandboxCharacter_PL_ST_001.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/StatusComponent_PL_ST_001.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogInputGateTicket, Log, All);

// Sets default values
ASandboxCharacter_PL_ST_001::ASandboxCharacter_PL_ST_001()
{	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	StatusComponent = CreateDefaultSubobject<UStatusComponent_PL_ST_001>(FName("StatusComponent"));
}

void ASandboxCharacter_PL_ST_001::ConsumeAxis2D_Move(const FVector2D& Value)
{
	// 이동은 기본적으로 CharacterMovement가 네트워크 처리(입력은 로컬), 하지만
	// Stun은 StatusComponent가 서버에서 DisableMovement 하므로 최종 진리는 서버가 잡음.
	AddMovementInput(GetActorForwardVector(), Value.Y);
	AddMovementInput(GetActorRightVector(), Value.X);
}

void ASandboxCharacter_PL_ST_001::ConsumeTrigger_Attack_Pressed()
{
	// 클라에서 호출될 수 있음 -> 서버로 최종 요정
	if (HasAuthority())
	{
		DoAttackPressed_ServerAuth();
	}
	else
	{
		Server_AttackPressed();
	}
}

void ASandboxCharacter_PL_ST_001::ConsumeTrigger_Attack_Released()
{
	if (HasAuthority())
	{
		DoAttackReleased_ServerAuth();
	}
	else
	{
		Server_AttackReleased();
	}
}

void ASandboxCharacter_PL_ST_001::Server_AttackPressed_Implementation()
{
	DoAttackPressed_ServerAuth();
}

void ASandboxCharacter_PL_ST_001::Server_AttackReleased_Implementation()
{
	DoAttackReleased_ServerAuth();
}

void ASandboxCharacter_PL_ST_001::DoAttackPressed_ServerAuth()
{
	if (!StatusComponent) return;
	UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackPressed IsStunned? %d"), StatusComponent->IsStunned());
	
	// 서버 최종 판정(클라 신뢰 금지)
	if (StatusComponent && StatusComponent->IsStunned())
	{
		UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackPressed rejected: Stunned"));
		return;
	}
	
	UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackPressed accepted"));
}

void ASandboxCharacter_PL_ST_001::DoAttackReleased_ServerAuth()
{
	if (!StatusComponent) return;
	UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackReleased IsStunned? %d"), StatusComponent->IsStunned());	
	
	if (StatusComponent && StatusComponent->IsStunned())
	{
		UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackReleased rejected: Stunned"));
		return;
	}
	
	UE_LOG(LogInputGateTicket, Warning, TEXT("[Server] AttackReleased accepted"));
}
