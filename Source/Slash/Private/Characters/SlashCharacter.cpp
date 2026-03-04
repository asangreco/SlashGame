#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LockOnComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Components/AttributeComponent.h"

#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/HealthPickup.h"



ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 450.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	AddMappingContext();

	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();

}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGoldCountText(0);
				SlashOverlay->SetSoulsCountText(0);
			}
		}
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attributes && SlashOverlay)
	{
		Attributes->RegenHealth(DeltaTime);
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

	// Sync CombatTarget with lock-on target for Motion Warping
	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		SetCombatTarget(LockOnComponent->GetCurrentTarget());
	}
	else
	{
		ClearCombatTarget();
	}

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASlashCharacter::Interaction);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ASlashCharacter::LockOn);
		EnhancedInputComponent->BindAction(CycleTargetLeftAction, ETriggerEvent::Started, this, &ASlashCharacter::CycleTargetLeft);
		EnhancedInputComponent->BindAction(CycleTargetRightAction, ETriggerEvent::Started, this, &ASlashCharacter::CycleTargetRight);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ASlashCharacter::Dodge);
	}
}

void ASlashCharacter::Jump()
{
	if (IsUnoccupied() && IsAlive())
	{
		Super::Jump();
	}

}

void ASlashCharacter::SetOverlappingItem(AActor* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSoulAmount());
		
		// Check if the player leveled up
		if (Attributes->CheckAndHandleLevelUp())
		{
			AddToDamageModifier(0.05f);
			// Update health bar to show new max health and full health
			SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
		}
		
		SlashOverlay->SetSoulsCountText(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGoldCountText(Attributes->GetGold());
	}
}

void ASlashCharacter::AddHealth(AHealthPickup* HealthPickup)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddHealth(HealthPickup->GetHealthAmount());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReacting;
	}


	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	DoComboAttack = false;
	DoThirdComboAttack = false;

	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ToggleLockOn();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	LastMovementInput = MovementVector;
	
	if (Controller && ActionState == EActionState::EAS_Unoccupied && IsAlive())
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, MovementVector.Y);
		AddMovementInput(Right, MovementVector.X);
	}

}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	if (LockOnComponent && LockOnComponent->IsLockedOn()) return;

	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void ASlashCharacter::Interaction()
{
	if (!IsAlive()) return;
	// Check if overlapping with a weapon and equip it
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		//if (!CanEquip()) return;
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
			CharacterState = ECharacterState::ECS_Unequipped;
		}
		EquipWeapon(OverlappingWeapon);
	}
	// If not overlapping with a weapon, check if overlapping with interactible item and interact with it if possible
	else if (OverlappingItem && OverlappingItem->Implements<UInteractibleInterface>())
	{
		IInteractibleInterface::Execute_Interact(OverlappingItem, this);
	}
	// If not overlapping with item, check if can disarm or arm 
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::LockOn()
{
	if (LockOnComponent && IsAlive())
	{
		LockOnComponent->ToggleLockOn();
	}
}

void ASlashCharacter::CycleTargetLeft()
{
	if (LockOnComponent && IsAlive())
	{
		LockOnComponent->CycleLockOnTarget(false);
	}
}

void ASlashCharacter::CycleTargetRight()
{
	if (LockOnComponent && IsAlive())
	{
		LockOnComponent->CycleLockOnTarget(true);
	}
}

void ASlashCharacter::Dodge()
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (!IsAlive()) return;
	if (!HasEnoughStamina()) return;
	
	HandleDodgeDirection();
	
	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodging;
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

}

void ASlashCharacter::HandleDodgeDirection()
{
	// Orient character to dodge direction based on input
	if (LastMovementInput.Size() > 0.1f && Controller)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Calculate dodge direction in world space
		const FVector DodgeDirection = (Forward * LastMovementInput.Y + Right * LastMovementInput.X).GetSafeNormal();

		// Set character rotation to face dodge direction
		if (DodgeDirection.Size() > 0.1f)
		{
			const FRotator NewRotation = DodgeDirection.Rotation();
			SetActorRotation(FRotator(0.f, NewRotation.Yaw, 0.f));
		}
	}
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() >= Attributes->GetDodgeCost();
}

void ASlashCharacter::AddMappingContext()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), FName("GripLocation"), this, this);

	IsTwoHanding = Weapon->IsTwoHanded();

	if (IsTwoHanding)
	{
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		AttackMontage = TwoHandedAttackMontage;
		EquipMontage = TwoHandedEquipMontage;
	}
	else if (!IsTwoHanding)
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		AttackMontage = OneHandedAttackMontage;
		EquipMontage = OneHandedEquipMontage;
	}
	else
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		AttackMontage = OneHandedAttackMontage;
		EquipMontage = OneHandedEquipMontage;
	}

	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
	
}

void ASlashCharacter::PlayAttackMontage()
{
	Super::PlayAttackMontage();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		PlayMontageSection(AttackMontage, GetAttackMontageSectionName());
	}
}

FName ASlashCharacter::GetAttackMontageSectionName()
{
	FName SelectionName = FName();
	if (!IsTwoHanding)
	{
		if (DoComboAttack)
		{
			EquippedWeapon->SetDamage(EquippedWeapon->GetBaseDamage() + 10 * DamageModifier);
			SelectionName = FName("Attack2");
			DoComboAttack = false;
		}
		else
		{
			EquippedWeapon->SetDamage(EquippedWeapon->GetBaseDamage() * DamageModifier);
			SelectionName = FName("Attack1");
			DoComboAttack = true;
		}
	}
	if (IsTwoHanding)
	{
		if (DoThirdComboAttack)
		{
			EquippedWeapon->SetDamage(EquippedWeapon->GetBaseDamage() + 20 * DamageModifier);
			SelectionName = FName("Attack3");
			DoThirdComboAttack = false;
		}
		else
			if (DoComboAttack)
			{
				EquippedWeapon->SetDamage(EquippedWeapon->GetBaseDamage() + 10 * DamageModifier);
				SelectionName = FName("Attack2");
				DoComboAttack = false;
				DoThirdComboAttack = true;
			}
			else
			{
				EquippedWeapon->SetDamage(EquippedWeapon->GetBaseDamage() * DamageModifier);
				SelectionName = FName("Attack1");
				DoComboAttack = true;
			}
	}
	return SelectionName;
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped && IsAlive();
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	//Play death montage
	PlayMontageSection(DeathMontage, GetRandMontageSectionName(DeathMontage));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::DodgeIFramesStart()
{
	// This function can be called from the dodge montage to enable i-frames during the dodge animation
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASlashCharacter::DodgeIFramesEnd()
{
	// This function can be called from the dodge montage to disable i-frames after the dodge animation
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped && EquipMontage;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

bool ASlashCharacter::CanEquip()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && !EquippedWeapon;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"), FName("GripLocation"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		IsTwoHanding = EquippedWeapon->IsTwoHanded();

		if (IsTwoHanding)
		{
			CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
			EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"), FName("GripLocation"));
		}
		else
		{
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"), FName("GripLocation"));
		}
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));

	// Set the correct character state based on weapon type
	if (EquippedWeapon->IsTwoHanded())
	{
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
	}
	else
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	}

	ActionState = EActionState::EAS_Equipping;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_Equipping;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}


