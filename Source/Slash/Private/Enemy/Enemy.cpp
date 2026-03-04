#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Items/Weapons/Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Items/Soul.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	// Setting character movement component so that the character rotates in the direction of movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->HearingThreshold = 1400.f;
	PawnSensing->LOSHearingThreshold = 2800.f;
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);	// Bind the PawnSeen function to the OnSeePawn event of the PawnSensing component, so that when the enemy sees a pawn, it will call the PawnSeen function
	InitializeEnemy();

	Tags.Add(FName("Enemy"));

}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If the enemy is in combat and not dead, check the combat target and move to it, otherwise if the enemy is patrolling, check the patrol target
	if (EnemyState > EEnemyState::EES_Patrolling && !IsDead())
	{
		CheckCombatTarget();
		MoveToTarget(CombatTarget);
	}
	else if (IsPatrolling())
	{ 
		CheckPatrolTarget(); 
	}
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	HideHealthBar();
	MoveToTarget(PatrolTarget);
	SpawnDefaultWeapon();
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();

	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), FName("GripLocation"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}
// Called from Weapon.cpp's OnBoxOverlap function, with it passing in damage and other info
float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead()) return 0.f;
	HandleDamage(DamageAmount);
	// Set the combat target to the player character that hit the enemy, so that the enemy can target the player in combat after being hit
	CombatTarget = EventInstigator->GetPawn();
	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	// If the enemy is destroyed and it has an equipped weapon, destroy the weapon as well
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

// Implementation of function from IHitInterface, called when enemy is hit overlapping with weapon hitbox, from Weapon.cpp's OnBoxOverlap function
void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (!IsDead()) ShowHealthBar();
	if (IsDead()) return;
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	ClearPatrolTimer();
	ClearAttackTimer();
	StopAttackMontage();

	if (IsInsideAttackRadius())
	{
		if (!IsDead())
		{
			StartAttackTimer();
		}

	}
}

// Called in GetHit() when enemy health reaches 0
void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// Play the death montage and select a random section to play, then set the death pose enum to the corresponding value based on the section that was played
	if (AnimInstance && DeathMontage)
	{
		PlayMontageSection(DeathMontage, GetRandMontageSectionName(DeathMontage));
	}
	DisableCapsule();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetLifeSpan(DeathLifeSpan);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	UWorld* World = GetWorld();
	SpawnSoul(World);
}

void AEnemy::SpawnSoul(UWorld* World)
{
	if (World && SoulClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSoulAmount(Attributes->GetSouls());
			SpawnedSoul->SetOwner(this);
		}
	}
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();

}

bool AEnemy::CanAttack()
{
	return IsInsideAttackRadius() && !IsAttacking() && !IsDead() && !IsEngaging();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::PlayAttackMontage()
{
	Super::PlayAttackMontage();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AttackMontage)
	{
		PlayMontageSection(AttackMontage, GetRandMontageSectionName(AttackMontage));
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius())
	{
		// If Outside combat Radius, lose interest in player and return to patrolling, and hide health bar widget
		LoseInterest();
		ClearAttackTimer();
		if (!IsEngaging())
		{
			StartPatrolling();
		}
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaging())
		{
			ChaseTarget();
		}
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}

void AEnemy::CheckPatrolTarget()
{
	// If enemy in near the patrol target, choose a new patrol target and move to it, then wait for a time before moving to next patrol target
	if (inTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, FMath::RandRange(PatrolWaitMin,PatrolWaitMax));
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrolWalkSpeed;
	MoveToTarget(PatrolTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{

	return !inTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !inTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return inTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsPatrolling()
{
	return EnemyState == EEnemyState::EES_Patrolling;
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaging()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ChaseTarget()
{
	// If outside attack radius but inside combat radius and not already chasing, chase the player
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChaseWalkSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::StartAttackTimer()
{
	// Start a timer to call the Attack() function after a random amount of time between AttackMin and AttackMax, and set the enemy state to attacking
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::inTargetRange(AActor* Target, double AcceptanceRadius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= AcceptanceRadius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	// If there is no enemy controller or target, return out of the function
	if (EnemyController == nullptr || Target == nullptr) return;
	// Create a move request to the target and set the acceptance radius to 15 units, then call the MoveTo function
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AIMovementAcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	// Create a new array of valid patrol targets that excludes the current patrol target
	TArray<AActor*> ValidTargets;
	for (auto& Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
	// If there are valid targets to choose from, select a random one and return it, otherwise return nullptr
	if (ValidTargets.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, ValidTargets.Num() - 1);
		return ValidTargets[Selection];
	}

	return nullptr;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (SeenPawn->ActorHasTag("Dead")) return;
	const bool bShouldChaseTarget = 
		IsPatrolling() &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));
	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

