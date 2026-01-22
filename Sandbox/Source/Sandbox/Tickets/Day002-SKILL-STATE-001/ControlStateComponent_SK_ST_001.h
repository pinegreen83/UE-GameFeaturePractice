// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ControlStateComponent_SK_ST_001.generated.h"

UENUM(BlueprintType)
enum class EControlOverrideState : uint8
{
	None,
	Stunned,
	Cinematic,
	Dead
};

UENUM(BlueprintType)
enum class EControlReason : uint8
{
	MoveLock,
	InputLock,
	LookLock
};

USTRUCT(BlueprintType)
struct FControlBlockHandle
{
	GENERATED_BODY()
	
	UPROPERTY() int32 Id = INDEX_NONE;
	UPROPERTY() EControlReason Reason = EControlReason::MoveLock;

	bool IsValid() const { return Id != INDEX_NONE; }
};

USTRUCT(BlueprintType)
struct FControlSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bCanMove = true;
	UPROPERTY(BlueprintReadOnly) bool bCanInput = true;
	UPROPERTY(BlueprintReadOnly) bool bCanLook = true;

	UPROPERTY(BlueprintReadOnly) EControlOverrideState OverrideState = EControlOverrideState::None;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SANDBOX_API UControlStateComponent_SK_ST_001 : public UActorComponent
{
	GENERATED_BODY()

public:
	UControlStateComponent_SK_ST_001();

	// --- Reason(합성) API : "요청" ---
	FControlBlockHandle AddBlockReason(EControlReason Reason, const TCHAR* DebugTea = TEXT(""));
	void RemoveBlockReason(const FControlBlockHandle& Handle);
	
	// --- Override(강제) API ---
	void SetOverrideState(EControlOverrideState NewState);
	EControlOverrideState GetOverrideState() const { return OverrideState; }
	
	// --- Read-Only snapshot (AnimInstance/UI용) ---
	UFUNCTION(BlueprintCallable, Category = "Control")
	FControlSnapshot GetSnapshot() const { return Snapshot; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	EControlOverrideState OverrideState = EControlOverrideState::None;

	// Reason 합성: Reason별 카운트
	UPROPERTY()
	TMap<EControlReason, int32> ReasonCounts;
	
	UPROPERTY()
	FControlSnapshot Snapshot;
	
	int32 NextHandleId = 1;
	TMap<int32, EControlReason> LiveHandles;
	
	// 실제 적용(권한)은 여기서만
	auto RecomputeAndApply() -> void;
	
	// 실제 실행부
	void ApplyToPawn(const FControlSnapshot& NewSnapshot);
};
