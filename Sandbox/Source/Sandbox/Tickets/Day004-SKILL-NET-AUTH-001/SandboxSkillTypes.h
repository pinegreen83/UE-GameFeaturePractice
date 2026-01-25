#pragma once

#include "CoreMinimal.h"
#include "SandboxSkillTypes.generated.h"

UENUM(BlueprintType)
enum class ESandboxSkillRejectReason : uint8
{
	None UMETA(DisplayName = "None"),
	NotOwner,
	StateBlocked,
	NotEnoughStamina,
	Cooldown,
	InvalidData,
};

USTRUCT(BlueprintType)
struct FSandboxSkillIntent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName SkillId = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector_NetQuantize10 ClientOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector_NetQuantizeNormal ClientForward = FVector::ForwardVector;

	// (옵션) 예측용 키. 이번 티켓에선 사용안함.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 PredictionKey = 0;
};

USTRUCT(BlueprintType)
struct FSandboxSkillResult
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
	ESandboxSkillRejectReason RejectReason = ESandboxSkillRejectReason::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 PredictionKey = 0;
};