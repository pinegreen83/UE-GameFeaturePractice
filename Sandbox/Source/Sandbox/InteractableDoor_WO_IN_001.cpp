// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableDoor_WO_IN_001.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AInteractableDoor_WO_IN_001::AInteractableDoor_WO_IN_001()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);

	DoorMesh->SetIsReplicated(false);
}

void AInteractableDoor_WO_IN_001::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInteractableDoor_WO_IN_001, bIsOpen);
}

void AInteractableDoor_WO_IN_001::OnRep_IsOpen()
{
	ApplyDoorVisual();
}

void AInteractableDoor_WO_IN_001::ApplyDoorVisual()
{
	// 예시: 회전으로 열림/닫힘 표현
	const float Yaw = bIsOpen ? 90.f : 0.f;
	SetActorRotation(FRotator(0.f, Yaw, 0.f));
}

FText AInteractableDoor_WO_IN_001::GetInteractPromptText_Implementation() const
{
	return bIsOpen ? FText::FromString(TEXT("Close")) : FText::FromString(TEXT("Open"));
}

bool AInteractableDoor_WO_IN_001::CanInteract_Implementation(AActor* Interactor) const
{
	// 필요하면 "사용 중" 락 / 팀 제한 / 상태 제한 등을 여기에 추가
	return true;
}

void AInteractableDoor_WO_IN_001::ExecuteInteract_Implementation(AActor* Interactor)
{
	// 서버에서만 호출되어야 함. (InteractionComponent에서 보장)
	bIsOpen = !bIsOpen;
	ApplyDoorVisual(); // 서버 즉시 반영
	// 클라는 OnRep으로 반영
}