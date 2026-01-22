#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillTypes_SK_CO_001.generated.h"

UENUM(BlueprintType)
enum class ESkillActivationPolicy : uint8
{
	Active,
	Passive
};

USTRUCT(BlueprintType)
struct FSkillStat
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StatName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Stat;
};

USTRUCT(BlueprintType)
struct FSkillHeader_SKILL_CORE_001
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SkillId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Thumbnail;
};

USTRUCT(BlueprintType)
struct FActiveSkillSpec_SKILL_CORE_001
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Range = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cost = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> SFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UParticleSystem> VFX;

	// 확장용: (Key, Value) 방식, 타입 안정성은 나중에 Tag/Enum으로 교체 권장.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSkillStat> Params;
};

USTRUCT(BlueprintType)
struct FPassiveSkillSpec_SKILL_CORE_001
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSkillStat> StatModifiers;
};

USTRUCT(BlueprintType)
struct FSkillDataRow_SKILL_CORE_001 : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSkillHeader_SKILL_CORE_001 Header;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ESkillActivationPolicy Policy = ESkillActivationPolicy::Active;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FActiveSkillSpec_SKILL_CORE_001 Active;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FPassiveSkillSpec_SKILL_CORE_001 Passive;
};

// Controller가 만들어서 Component로 넘기는 의도(Intent)
USTRUCT(BlueprintType)
struct FSkillIntentData_SKILL_CORE_001
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantizeNormal AimDir = FVector::ForwardVector;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> SuggestedTarget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float ClientTimeSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct FSkillExecutionContext_SKILL_CORE_001
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InstigatorActor = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantizeNormal AimDir = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly)
	float TimestampSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasAuthority = false;
};

USTRUCT(BlueprintType)
struct FSkillCueData_SKILL_CORE_001
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName SkillId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InstigatorActor = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantizeNormal AimDir = FVector::ForwardVector;
};

USTRUCT(BlueprintType)
struct FCooldownEntry_SKILL_CORE_001
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName SkillId = NAME_None;
	
	UPROPERTY(BlueprintReadOnly)
	float EndTime = 0.f;
};