// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup_HealKit_WO_IN_001.h"

#include "Components/SphereComponent.h"

// Sets default values
APickup_HealKit_WO_IN_001::APickup_HealKit_WO_IN_001()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);

	Sphere->InitSphereRadius(70.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Sphere);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &APickup_HealKit_WO_IN_001::OnSphereBeginOverlap);
}

void APickup_HealKit_WO_IN_001::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSeep, const FHitResult& SweepResult)
{
	// 서버에서만 "획득 확정"
	if (!HasAuthority())
	{
		return;
	}
	
	if(bConsumed)
	{
		return;
	}
	
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}
	
	ConsumeBy(Pawn);
}

void APickup_HealKit_WO_IN_001::ConsumeBy(AActor* Consumer)
{
	if (bConsumed)
	{
		return;
	}
	bConsumed = true;

	// 여기서 Heal 적용: 프로젝트에 Health 시스템이 있다면 그 인터페이스/컴포넌트를 찾아 호출.
	// 예시로는 Consumer에 "AddHealth" 같은 함수를 두거나, HealthComponent를 찾아 호출하면 됨.

	// 상태 변화(사라짐)는 서버에서 처리-> 클라는 replication으로 사라짐을 봄.
	Destroy();
}
