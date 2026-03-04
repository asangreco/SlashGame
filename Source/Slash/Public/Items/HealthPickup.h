#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "HealthPickup.generated.h"

UCLASS()
class SLASH_API AHealthPickup : public AItem
{
	GENERATED_BODY()
	
public:	

	int32 GetHealthAmount() const { return HealthAmount; }
	void SetHealthAmount(int32 Amount) { HealthAmount = Amount; }

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditAnywhere, Category = "Pickup Properties")
	int32 HealthAmount;
	
	
};
