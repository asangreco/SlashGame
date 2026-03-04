#include "Items/Soul.h"
#include "Interfaces/InteractibleInterface.h"
#include "Kismet/KismetSystemLibrary.h"


void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const double LocationZ = GetActorLocation().Z;
	if (LocationZ > DesiredZLocation)
	{
		const FVector DeltaLocation = FVector(0.f, 0.f, DriftRate * DeltaTime);
		AddActorWorldOffset(DeltaLocation);
	}

}

void ASoul::BeginPlay()
{
	Super::BeginPlay();

	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = StartLocation - FVector(0.f, 0.f, 2000.f);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingleForObjects(this, StartLocation, EndLocation, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
	DesiredZLocation = HitResult.ImpactPoint.Z + 50.f;
}

void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractibleInterface* InteractibleInterface = Cast<IInteractibleInterface>(OtherActor);
	if (InteractibleInterface)
	{
		InteractibleInterface->AddSouls(this);
		SpawnPickupSystem(OtherActor);
		SpawnPickupSound();
		Destroy();
	}
	
}
