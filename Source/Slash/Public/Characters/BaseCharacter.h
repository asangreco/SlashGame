#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"


#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

protected:

	virtual void BeginPlay() override;

	/* Combat */ 

	virtual void Attack();
	UFUNCTION(BlueprintNativeEvent)
	void Die();
	virtual void HandleDamage(float DamageAmount);
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	UFUNCTION(BlueprintCallable)
	virtual bool CanAttack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	UFUNCTION(BlueprintCallable)
	void SetCombatTarget(AActor* Target) { CombatTarget = Target; }
	UFUNCTION(BlueprintCallable)
	void ClearCombatTarget() { CombatTarget = nullptr; }

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes | Combat")
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Actor Attributes | Combat")
	AActor* CombatTarget;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	double WarpTargetDistance = 75.f;

	/* Combat End */


	/* Animation, FX, and Montages */

	virtual void PlayAttackMontage();
	virtual void PlayDodgeMontage();
	void PlayHitReactMontage(const FName& SectionName);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	FName GetRandMontageSectionName(UAnimMontage* Montage);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void StopAttackMontage();


	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* DeathMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	USoundBase* HitSound;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	UParticleSystem* HitParticles;



	/* Animation, FX, and Montages End*/


	/* Components */

	void DisableCapsule();
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	UAttributeComponent* Attributes;

	/* Components End*/

private:	


	
	
};
