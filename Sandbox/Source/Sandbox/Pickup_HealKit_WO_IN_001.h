// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup_HealKit_WO_IN_001.generated.h"

UCLASS()
class SANDBOX_API APickup_HealKit_WO_IN_001 : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup_HealKit_WO_IN_001();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float HealAmount = 25.0f;
	
	// 레이스 방지(원자적 처리)
	bool bConsumed = false;
	
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSeep, const FHitResult& SweepResult);

	void ConsumeBy(AActor* Consumer);
};
