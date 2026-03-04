// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(GetRootComponent());

	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

	Damage = BaseDamage;

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	// Bind the OnBoxOverlap function to the OnComponentBeginOverlap event of the WeaponBox component
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}
// This function is called when the player equips the weapon, it sets the owner and instigator of the weapon, attaches the weapon mesh to the specified socket on the character's mesh, and plays an equip sound if one is set. It also disables collision on the sphere component and deactivates the embers effect if they exist
void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, FName WeaponSocketName, AActor* NewOwner, APawn* NewInstigator)
{
		ItemState = EItemState::EIS_Equipped;
		SetOwner(NewOwner);
		SetInstigator(NewInstigator);
		AttachMeshToSocket(InParent, InSocketName, WeaponSocketName);
		SetIsTwoHanded(IsTwoHanded());
		DisableSphereCollision();

		PlayEquipSound();
		DeactivateEmbers();
}
void AWeapon::DeactivateEmbers()
{
	if (ItemEffect)
	{
		ItemEffect->Deactivate();
	}
}
void AWeapon::DisableSphereCollision()
{
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}
// This function attaches the weapon mesh to the specified socket on the character's mesh and applies a compensation transform if the socket exists on the weapon mesh
void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName, const FName& WeaponSocketName)
{
	FAttachmentTransformRules TransformRule(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false);
	ItemMesh->AttachToComponent(InParent, TransformRule, InSocketName);
	if (ItemMesh->DoesSocketExist(WeaponSocketName))
		{
			FTransform HandleSocketTransform = ItemMesh->GetSocketTransform(WeaponSocketName, ERelativeTransformSpace::RTS_Component);
			FTransform CompensationTransform = HandleSocketTransform.Inverse();
			ItemMesh->SetRelativeTransform(CompensationTransform,false, nullptr, ETeleportType::None);
		}
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorIsSameType(OtherActor))
	{
		return;
	}

	FHitResult BoxHit;
	BoxTrace(BoxHit);
	if (BoxHit.GetActor())
	{
		if (ActorIsSameType(BoxHit.GetActor()))
		{
			return;
		}
		if (BoxHit.GetActor() == GetOwner())
		{
			return;
		}
		// Calls Hit Actor's TakeDamage function, passing in the damage value and other relevant information
		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());

		ExecuteGetHit(BoxHit);

		//Call the BlueprintImplementableEvent to create fields at the impact point of the hit
		CreateFields(BoxHit.ImpactPoint);

	}

}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	// Check if the hit actor implements the IHitInterface and call the GetHit function if it does
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
	if (HitInterface)
	{
		// Call the GetHit function on the hit actor, passing in the impact point of the hit
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());
	for (AActor* Actor : IgnoredActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}
	// Perform a box trace to detect any actors in the path of the weapon's swing
	UKismetSystemLibrary::BoxTraceSingle(this, Start, End, BoxTraceExtent, BoxTraceStart->GetComponentRotation(), ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, BoxHit, true);
	IgnoredActors.AddUnique(BoxHit.GetActor());
}


