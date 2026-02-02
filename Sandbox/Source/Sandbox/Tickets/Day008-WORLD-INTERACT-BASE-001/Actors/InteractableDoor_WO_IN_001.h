// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tickets/Day008-WORLD-INTERACT-BASE-001/Interaction/Interactable_WO_IN_001.h"
#include "InteractableDoor_WO_IN_001.generated.h"

UCLASS()
class SANDBOX_API AInteractableDoor_WO_IN_001 : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AInteractableDoor_WO_IN_001();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DoorMesh;
	
	// 문 상태는 모든 클라가 봐야 하므로 Replicate
	UPROPERTY(ReplicatedUsing=OnRep_IsOpen)
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_IsOpen();

	void ApplyDoorVisual();

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// IInteractable
	virtual FText GetInteractPromptText_Implementation() const override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void ExecuteInteract_Implementation(AActor* Interactor) override;
};
