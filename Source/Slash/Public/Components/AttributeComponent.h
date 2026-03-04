#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenHealth(float DeltaTime);
	void RegenStamina(float DeltaTime);

protected:
	virtual void BeginPlay() override;

private:
	//Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float Health;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float Stamina;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	int32 Gold;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	int32 Souls;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	int32 SoulsPerLevel = 100;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float HealthIncreasePerLevel = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float DodgeCost = 14.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float HealthRegenRate = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes | Attribute Component")
	float StaminaRegenRate = 6.f;

public:

	void ReceiveDamage(float DamageAmount);
	void UseStamina(float StaminaCost);
	void AddGold(int32 GoldAmount);
	void AddSouls(int32 SoulAmount);
	bool CheckAndHandleLevelUp();


	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float AddHealth(float HealthAmount) { return Health = FMath::Clamp(Health + HealthAmount, 0.f, MaxHealth); }
	FORCEINLINE float RemoveHealth(float HealthAmount) { return Health = FMath::Clamp(Health - HealthAmount, 0.f, MaxHealth); }
	FORCEINLINE void SetHealth(float HealthAmount) { Health = FMath::Clamp(HealthAmount, 0.f, MaxHealth); }
	FORCEINLINE float GetHealthPercent() const { return Health / MaxHealth; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }
	FORCEINLINE float GetStaminaPercent() const { return Stamina / MaxStamina; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }



	bool isAlive();

};
