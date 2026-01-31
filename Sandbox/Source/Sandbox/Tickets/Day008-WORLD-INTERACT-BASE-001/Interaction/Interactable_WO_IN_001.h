#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable_WO_IN_001.generated.h"

UINTERFACE(BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class IInteractable
{
	GENERATED_BODY()

public:
	// 서버 검증 전에 클라 UI를 위해 호출해도 되는 가벼운 힌트.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	FText GetInteractPromptText() const;

	// 서버에서 최종 실행 가능 여부 판단(락/상태 등).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	bool CanInteract(AActor* Interactor) const;

	// 서버에서만 호출되어야 함. 실제 상태 변화 수행.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void ExecuteInteract(AActor* Interactor);
};