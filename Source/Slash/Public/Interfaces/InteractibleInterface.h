#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "InteractibleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractibleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IInteractibleInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Interact(class ASlashCharacter* Character);

	virtual void SetOverlappingItem(class AActor* Item);

	virtual void AddSouls(class ASoul* Soul);

	virtual void AddGold(class ATreasure* Treasure);

	virtual void AddHealth(class AHealthPickup* HealthPickup);
};
