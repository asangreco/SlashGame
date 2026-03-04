#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()



public: 
	AEnemy();
	/* AActor Override */

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	/*/AActor Override*/

	/* IHitInterface Override */

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	/*/IHitInterface Override*/

	UFUNCTION(BlueprintPure)
	bool IsDead();

protected:
	/* AActor Override */

	virtual void BeginPlay() override;

	/*/AActor Override*/

	/* Initializing Enemy */

	void InitializeEnemy();
	void SpawnDefaultWeapon();

	/*Initializing Enemy End*/

	/* ABaseCharacter Override */

	void Die_Implementation();
	void SpawnSoul(UWorld* World);
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void AttackEnd() override;
	virtual void PlayAttackMontage() override;

	/* /ABaseCharacter Override */


	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose;

private:

	/* AI Behavior*/

	void CheckCombatTarget();
	void CheckPatrolTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsPatrolling();
	bool IsChasing();
	bool IsAttacking();
	bool IsEngaging();
	void ClearPatrolTimer();
	void ChaseTarget();
	void StartAttackTimer();
	void ClearAttackTimer();
	bool inTargetRange(AActor* Target, double AcceptanceRadius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);//Callback for OnPawnSeen from PawnSensingComponent

	FTimerHandle PatrolTimer;
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	double CombatRadius = 1000.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	double AttackRadius = 150.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	float AttackMin = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	float AttackMax = 1.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Combat")
	float DeathLifeSpan = 10.f;

	UPROPERTY()
	class AAIController* EnemyController;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	AActor* PatrolTarget;//Current Patrol Target, set in Editor
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	TArray<AActor*> PatrolTargets;	//Array of All Patrol Targets
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	double AIMovementAcceptanceRadius = 45.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	double PatrolRadius = 200.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	float PatrolWaitMin = 5.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	float PatrolWaitMax = 10.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	float PatrolWalkSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | AI Behavior")
	float ChaseWalkSpeed = 300.f;

	/* End AI Behavior*/

	/* Components */

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	UHealthBarComponent* HealthBarWidget;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	UPawnSensingComponent* PawnSensing;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	TSubclassOf<class AWeapon> WeaponClass;

	/* End Components*/

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	TSubclassOf<class ASoul> SoulClass;

protected:

};
