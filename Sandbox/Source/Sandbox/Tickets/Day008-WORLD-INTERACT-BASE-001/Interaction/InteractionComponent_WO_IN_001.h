// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent_WO_IN_001.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractFocusChanged, AActor*, NewFocus, FText, Prompt);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOX_API UInteractionComponent_WO_IN_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent_WO_IN_001();

	// 클라 UI용: 현재 상호작용 후보가 바뀔 때 UI가 구독
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnInteractFocusChanged OnFocusChanged;

	// Character가 Overlap에서 호출
	void NotifyBeginOverlap(AActor* OtherActor);
	void NotifyEndOverlap(AActor* OtherActor);

	// 입력으로 호출 (클라에서)
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void AttemptInteract();

protected:
	virtual void BeginPlay() override;

	// 서버에 상호작용 요청
	UFUNCTION(Server, Reliable)
	void Server_RequestInteract(AActor* Target);
	
	bool ValidateInteract_Server(AActor* Target) const;

	// 현재 포커스(클라 UI용). 서버 authoritative 상태와는 별개.
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> FocusActor;

	// 서버 검증용
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float MaxInteractDistance = 250.0f;
};
