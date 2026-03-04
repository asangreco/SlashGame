#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Interfaces/InteractibleInterface.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class UAnimMontage;
class ULockOnComponent;
class USlashOverlay;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IInteractibleInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual void SetOverlappingItem(class AActor* Item) override;
	virtual void AddSouls(class ASoul* Soul) override;
	virtual void AddGold(class ATreasure* Treasure) override;
	virtual void AddHealth(class AHealthPickup* HealthPickup) override;

	bool IsUnoccupied();

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void SetHUDHealth();

	float GetDamageModifier() { return DamageModifier; }
	void SetDamageModifier(float Modifier) { DamageModifier = Modifier; }
	void AddToDamageModifier(float Modifier) { DamageModifier += Modifier; }

protected:
	virtual void BeginPlay() override;

	void InitializeSlashOverlay();


	/* Callbacks for Input */

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interaction();
	void LockOn();
	void CycleTargetLeft();
	void CycleTargetRight();
	void Dodge();

	void HandleDodgeDirection();

	bool HasEnoughStamina();
	
	/* Callbacks for Input End*/


	/* Input */

	void AddMappingContext();

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* MovementAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* CycleTargetLeftAction;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* CycleTargetRightAction;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Input")
	UInputAction* DodgeAction;

	/* Input End */


	/* Combat */

	void EquipWeapon(AWeapon* Weapon);
	virtual void Attack() override;
	virtual void PlayAttackMontage() override;
	FName GetAttackMontageSectionName();
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	void Die_Implementation();
	void PlayEquipMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	bool CanDisarm();
	UFUNCTION(BlueprintCallable)
	bool CanArm();
	UFUNCTION(BlueprintCallable)
	bool CanEquip();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	UFUNCTION(BlueprintCallable)
	void Arm();
	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();
	

	UPROPERTY(BlueprintReadWrite, Category = "Actor Attributes | Combat")
	bool DoComboAttack = false;
	UPROPERTY(BlueprintReadWrite, Category = "Actor Attributes | Combat")
	bool DoThirdComboAttack = false;
	UPROPERTY(BlueprintReadWrite, Category = "Actor Attributes | Combat")
	bool IsTwoHanding = false;

	float DamageModifier = 1.f;

	/* Combat End */

private:

	/* Enums */

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	/* Enums End */ 


	/* Components */

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;
	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;
	UPROPERTY(VisibleInstanceOnly)
	AActor* OverlappingItem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actor Attributes | Combat", meta = (AllowPrivateAccess = "true"))
	ULockOnComponent* LockOnComponent; 

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	/* Components End */

	/* Movement */

	FVector2D LastMovementInput;

	/* Movement End */

	/* Montages */

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes | Combat")
	UAnimMontage* EquipMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* OneHandedEquipMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* TwoHandedEquipMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* OneHandedAttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Actor Attributes | Combat")
	UAnimMontage* TwoHandedAttackMontage;

	/* Montages End */

public:
	//FORCEINLINE void SetOverlappingItem(AActor* Actor) { OverlappingItem = Actor; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE ULockOnComponent* GetLockOnComponent() const { return LockOnComponent; }
};
