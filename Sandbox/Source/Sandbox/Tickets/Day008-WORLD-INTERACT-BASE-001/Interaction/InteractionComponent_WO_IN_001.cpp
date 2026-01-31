// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day008-WORLD-INTERACT-BASE-001/Interaction/InteractionComponent_WO_IN_001.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Tickets/Day008-WORLD-INTERACT-BASE-001/Interaction/Interactable_WO_IN_001.h"

// Sets default values for this component's properties
UInteractionComponent_WO_IN_001::UInteractionComponent_WO_IN_001()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false); // 이 컴포넌트는 "요청 생성/클라 UI" 중심이라 굳이 복제 필요 없음
}

void UInteractionComponent_WO_IN_001::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent_WO_IN_001::NotifyBeginOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	// UI 후보는 클라 로컬에서만 관리해도 됨. (서버에서도 실행해도 문제는 없지만 목적이 UI)
	if (!OtherActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return;
	}

	// 더 정교하게 하려면 거리/시야 기반으로 "가장 적합한 후보"를 선택.
	FocusActor = OtherActor;

	const FText Prompt = IInteractable::Execute_GetInteractPromptText(OtherActor);
	OnFocusChanged.Broadcast(OtherActor, Prompt);
}

void UInteractionComponent_WO_IN_001::NotifyEndOverlap(AActor* OtherActor)
{
	if (!OtherActor)
	{
		return;
	}

	if (FocusActor.Get() == OtherActor)
	{
		FocusActor == nullptr;
		OnFocusChanged.Broadcast(nullptr, FText::GetEmpty());
	}
}

void UInteractionComponent_WO_IN_001::AttemptInteract()
{
	AActor* Target = FocusActor.Get();
	if (!Target)
	{
		return;
	}

	// 의도는 클라에서 생성 -> 서버로 단발 요청
	if (GetOwnerRole() == ROLE_Authority)
	{
		// 싱글/서버 플레이 시에도 동작
		Server_RequestInteract(Target);
		return;
	}

	Server_RequestInteract(Target);
}

void UInteractionComponent_WO_IN_001::Server_RequestInteract_Implementation(AActor* Target)
{
	if (!ValidateInteract_Server(Target))
	{
		return;
	}

	// 서버에서 최종 승인: 오브젝트가 상태 변화를 수행
	IInteractable::Execute_ExecuteInteract(Target, GetOwner());
}

bool UInteractionComponent_WO_IN_001::ValidateInteract_Server(AActor* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	if (!Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return false;
	}

	// 1) 거리 검증
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	if (FVector::DistSquared(OwnerLocation, TargetLocation) > FMath::Square(MaxInteractDistance))
	{
		return false;
	}
	
	// 2) 오브젝트 상태(락/이미 사용됨 등)
	if (!IInteractable::Execute_CanInteract(Target, GetOwner()))
	{
		return false;
	}

	return true;
}
