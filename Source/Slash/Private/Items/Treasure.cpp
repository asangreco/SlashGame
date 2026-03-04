
#include "Items/Treasure.h"
#include "Interfaces/InteractibleInterface.h"

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractibleInterface* InteractibleInterface = Cast<IInteractibleInterface>(OtherActor);
	if (InteractibleInterface)
	{
		InteractibleInterface->AddGold(this);
		SpawnPickupSound();
		Destroy();
	}

}
