// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickets/Day009-UI-FEEDBACK-BASE-001/SandboxSkillTypes_UI_FE_001.h"
#include "FeedbackSubsystem_UI_FE_001.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONSandboxFeedbackEvent, const FSandboxFeedbackEvent&, Event);

UCLASS()
class SANDBOX_API UFeedbackSubsystem_UI_FE_001 : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UI / Debug sink들이 여기에 구독
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Feedback")
	FONSandboxFeedbackEvent OnFeedback;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Feedback")
	void Publish(const FSandboxFeedbackEvent& Event)
	{
		OnFeedback.Broadcast(Event);
	}
};
