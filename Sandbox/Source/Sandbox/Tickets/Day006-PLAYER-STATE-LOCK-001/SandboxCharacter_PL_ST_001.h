// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Tickets/Day006-PLAYER-STATE-LOCK-001/InputConsumerInterface_PL_ST_001.h"
#include "SandboxCharacter_PL_ST_001.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStatusComponent_PL_ST_001;

UCLASS()
class SANDBOX_API ASandboxCharacter_PL_ST_001 : public ACharacter, public IInputConsumerInterface_PL_ST_001
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	ASandboxCharacter_PL_ST_001();

	// IInputConsumerInterface
	virtual void ConsumeAxis2D_Move(const FVector2D& Value) override;
	virtual void ConsumeTrigger_Attack_Pressed() override;
	virtual void ConsumeTrigger_Attack_Released() override;

	UStatusComponent_PL_ST_001* GetStatusComponent() const { return StatusComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStatusComponent_PL_ST_001> StatusComponent;

private:
	UFUNCTION(Server, Reliable)
	void Server_AttackPressed();

	UFUNCTION(Server, Reliable)
	void Server_AttackReleased();

	void DoAttackPressed_ServerAuth();
	void DoAttackReleased_ServerAuth();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
