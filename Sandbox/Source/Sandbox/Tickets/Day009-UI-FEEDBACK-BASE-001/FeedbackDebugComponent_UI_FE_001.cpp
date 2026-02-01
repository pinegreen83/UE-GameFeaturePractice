// Fill out your copyright notice in the Description page of Project Settings.


#include "Tickets/Day009-UI-FEEDBACK-BASE-001/FeedbackDebugComponent_UI_FE_001.h"
#include "Engine/Engine.h"
#include "Tickets/Day009-UI-FEEDBACK-BASE-001/FeedbackSubsystem_UI_FE_001.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogSandboxFeedback);

UFeedbackDebugComponent_UI_FE_001::UFeedbackDebugComponent_UI_FE_001()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFeedbackDebugComponent_UI_FE_001::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UFeedbackSubsystem_UI_FE_001* Subsystem = GI->GetSubsystem<UFeedbackSubsystem_UI_FE_001>())
		{
			Subsystem->OnFeedback.AddDynamic(this, &UFeedbackDebugComponent_UI_FE_001::HandleFeedback);
		}
	}
}

void UFeedbackDebugComponent_UI_FE_001::HandleFeedback(const FSandboxFeedbackEvent& Event)
{
	const TCHAR* ResultStr = (Event.Result == ESandboxFeedbackResult::Success) ? TEXT("SUCCESS") : TEXT("FAIL");
	const FString ReasonStr = ToReasonString(Event.Reason);
	const FString StageStr = ToStageString(Event.Stage);

	const FString Msg = FString::Printf(
		TEXT("[%s][%s] Skill = %s Reason = %s"),
		ResultStr, *StageStr, *Event.SkillId.ToString(), *ReasonStr);

	UE_LOG(LogSandboxFeedback, Log, TEXT("%s"), *Msg);
	
	if (GEngine)
	{
		const float Duration = 1.5f;
		const FColor Color = (Event.Result == ESandboxFeedbackResult::Success) ? FColor::Green : FColor::Red;
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Msg);
	}
}

FString UFeedbackDebugComponent_UI_FE_001::ToReasonString(ESandboxSkillRejectReason_UI Reason) const
{
	FString S = UEnum::GetValueAsString(Reason);
	S.RemoveFromStart(TEXT("ESandboxSkillRejectReason_UI::"));
	return S;
}

FString UFeedbackDebugComponent_UI_FE_001::ToStageString(ESandboxFeedbackStage Stage) const
{
	FString S = UEnum::GetValueAsString(Stage);
	S.RemoveFromStart(TEXT("ESandboxFeedbackStage::"));
	return S;
}
