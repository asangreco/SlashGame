#include "Characters/BaseCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"

#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoredActors.Empty();
	}
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead"));
}

void ABaseCharacter::PlayAttackMontage()
{
	// This function is meant to be overridden in child classes, so it is left empty here.
}

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(DodgeMontage, FName(TEXT("Default")));
}

// This function plays the hit react montage and jumps to the section that corresponds to the direction of the hit
void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered = FVector(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward| * |ToHit| * cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Theta = acos(CosTheta)
	double Theta = FMath::Acos(CosTheta);
	// Convert Theta from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// If cross product is up, then the hit is on the right, otherwise it is on the left
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);

	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName SectionName = "FromBack";
	if (Theta >= -45.f && Theta < 45.f)
	{
		SectionName = "FromFront";
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		SectionName = "FromRight";
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		SectionName = "FromLeft";
	}
	PlayHitReactMontage(SectionName);

}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToCharacter = (CombatTargetLocation - Location).GetSafeNormal();
	TargetToCharacter *= WarpTargetDistance;

	return CombatTargetLocation + TargetToCharacter;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}

	return FVector();
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

FName ABaseCharacter::GetRandMontageSectionName(UAnimMontage* Montage)
{
	const int32 Selection = FMath::RandRange(0, Montage->GetNumSections() - 1);
	FName SectionName = Montage->GetSectionName(Selection);
	return SectionName;
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ABaseCharacter::CanAttack()
{
	// This function is meant to be overridden in child classes, so it returns false here.
	return false;
}

void ABaseCharacter::AttackEnd()
{
	// This function is meant to be overridden in child classes, so it is left empty here.
}

void ABaseCharacter::DodgeEnd()
{
	// This function is meant to be overridden in child classes, so it is left empty here.
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->isAlive();
}