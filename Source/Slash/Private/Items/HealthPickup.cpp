#include "Items/HealthPickup.h"
#include "Interfaces/InteractibleInterface.h"

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractibleInterface* InteractibleInterface = Cast<IInteractibleInterface>(OtherActor);
	if (InteractibleInterface)
	{
		InteractibleInterface->AddHealth(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}
