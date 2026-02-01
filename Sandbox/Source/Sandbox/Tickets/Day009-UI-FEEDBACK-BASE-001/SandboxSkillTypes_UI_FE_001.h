#pragma once

#include "CoreMinimal.h"
#include "SandboxSkillTypes_UI_FE_001.generated.h"

UENUM(BlueprintType)
enum class ESandboxSkillRejectReason_UI : uint8
{
	None UMETA(DisplayName = "None"),
	NotOwner,
	StateBlocked,
	NotEnoughStamina,
	Cooldown,
	InvalidData,
};

// === Day009: Feedback Types ===

UENUM(BlueprintType)
enum class ESandboxFeedbackResult : uint8
{
	Success UMETA(DisplayName = "Success"),
	Fail	UMETA(DisplayName = "Fail"),
};

UENUM(BlueprintType)
enum class ESandboxFeedbackStage : uint8
{
	None	 UMETA(DisplayName = "None"),
	Review	 UMETA(DisplayName = "Review"),		// "스킬 검토"
	Validate UMETA(DisplayName = "Validate"), 	// "타당성 확인"
	Apply	 UMETA(DisplayName = "Apply"),		// "적용 검토"
	Commit 	 UMETA(DisplayName = "Commit")		// "Commit"
};

USTRUCT(BlueprintType)
struct FSandboxFeedbackEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESandboxFeedbackResult Result = ESandboxFeedbackResult::Fail;

	// 실패 시 원인. 성공이면 None 권장.
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESandboxSkillRejectReason_UI Reason = ESandboxSkillRejectReason_UI::None;

	// 디버그용: 어느 단계에서 실패/성공 확정됐는지
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESandboxFeedbackStage Stage = ESandboxFeedbackStage::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName SkillId = NAME_None;
};

USTRUCT(BlueprintType)
struct FSandboxSkillIntent_UI
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName SkillId = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector_NetQuantize10 ClientOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector_NetQuantizeNormal ClientForward = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 PredictionKey = 0;
};

USTRUCT(BlueprintType)
struct FSandboxSkillResult_UI
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bApproved = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName SkillId = NAME_None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FVector_NetQuantize10 ServerOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FVector_NetQuantizeNormal ServerForward = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESandboxSkillRejectReason_UI RejectReason = ESandboxSkillRejectReason_UI::None;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 PredictionKey = 0;
};