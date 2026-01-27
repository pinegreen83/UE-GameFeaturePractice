#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillTypes_SK_CO_RE.generated.h"

UENUM(BlueprintType)
enum class ESkillFailReason : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidSkillId UMETA(DisplayName = "InvalidSkillId"),
	MissingDependency UMETA(DisplayName = "MissingDependency"),
	NoAuthority UMETA(DisplayName = "NoAuthority"),
	InvalidState UMETA(DisplayName = "InvalidState"),
	Cooldown UMETA(DisplayName = "Cooldown"),
	NotEnoughResource UMETA(DisplayName = "NotEnoughResource"),
};

USTRUCT(BlueprintType)
struct FSkillIntent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantizeNormal Forward = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite)
	uint8 ClientSequence = 0; // 연타, 중복 요청 디버그용(옵션)
};

USTRUCT(BlueprintType)
struct FSkillValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bCanActivate = false;

	UPROPERTY(BlueprintReadOnly)
	ESkillFailReason FailReason = ESkillFailReason::None;

	// 디버그/UI용 (필요한 것만)
	UPROPERTY(BlueprintReadOnly)
	float CooldownRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float RequiredResource = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentResource = 0.f;
};

USTRUCT(BlueprintType)
struct FSkillResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly)
	ESkillFailReason FailReason = ESkillFailReason::None;

	UPROPERTY(BlueprintReadOnly)
	float CooldownRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float RequiredResource = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentResource = 0.f;
};

USTRUCT(BlueprintType)
struct FSkillRuntimeState
{
	GENERATED_BODY()

	// 서버 기준: 쿨다운 종료 "시간" (World->GetTimeSeconds 기반)
	UPROPERTY()
	float CooldownEndTime = 0.f;

	bool IsOnCooldown(const float ServerNow, float& OutRemaining) const
	{
		OutRemaining = FMath::Max(0.f, CooldownEndTime - ServerNow);
		return OutRemaining > 0.f;
	}
};

USTRUCT(BlueprintType)
struct FSkillDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SkillId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CooldownSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ResourceCost = 10.f;
	
	// 무거운 리소스는 여기 하드 레퍼런스 금지(필요하면 ResourceId/SoftPtr로 분리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ResourceId;
};

static FORCEINLINE const TCHAR* SK_ToString(ESkillFailReason Reason)
{
	switch (Reason)
	{
		case ESkillFailReason::None: return TEXT("None");
		case ESkillFailReason::InvalidSkillId: return TEXT("InvalidSkillId");
		case ESkillFailReason::MissingDependency: return TEXT("MissingDependency");
		case ESkillFailReason::NoAuthority: return TEXT("NoAuthority");
		case ESkillFailReason::InvalidState: return TEXT("InvalidState");
		case ESkillFailReason::Cooldown: return TEXT("Cooldown");
		case ESkillFailReason::NotEnoughResource: return TEXT("NotEnoughResource");
		default: return TEXT("Unknown");
	}
}