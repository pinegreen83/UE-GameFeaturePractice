// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tickets/Day009-UI-FEEDBACK-BASE-001/SandboxSkillTypes_UI_FE_001.h"
#include "FeedbackDebugComponent_UI_FE_001.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSandboxFeedback, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UFeedbackDebugComponent_UI_FE_001 : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFeedbackDebugComponent_UI_FE_001();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleFeedback(const FSandboxFeedbackEvent& Event);

	FString ToReasonString(ESandboxSkillRejectReason_UI Reason) const;
	FString ToStageString(ESandboxFeedbackStage Stage) const;
};
