#include "Components/AttributeComponent.h"


UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}
// Decreases health by specified damage amount, called in TakeDamage() in Enemy.cpp
void UAttributeComponent::ReceiveDamage(float DamageAmount)
{
	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);

}

void UAttributeComponent::UseStamina(float StaminaCost)
{
	Stamina = FMath::Clamp(Stamina - StaminaCost, 0.f, MaxStamina);
}

void UAttributeComponent::AddGold(int32 GoldAmount)
{
	Gold += GoldAmount;
}

void UAttributeComponent::AddSouls(int32 SoulAmount)
{
	Souls += SoulAmount;
}

bool UAttributeComponent::CheckAndHandleLevelUp()
{
	if (Souls >= SoulsPerLevel)
	{
		// Calculate how many levels to gain
		int32 LevelsGained = Souls / SoulsPerLevel;
		
		// Increase max health
		MaxHealth += HealthIncreasePerLevel * LevelsGained;
		
		//Heal half of total health on level up
		AddHealth(MaxHealth * 0.5f);
		
		// Subtract the souls used for leveling
		Souls -= (LevelsGained * SoulsPerLevel);

		//Increase the souls required for the next level up
		SoulsPerLevel += 50 * LevelsGained;
		
		return true;
	}
	return false;
}

bool UAttributeComponent::isAlive()
{
	return Health>0.f;
}


void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

void UAttributeComponent::RegenHealth(float DeltaTime)
{
	Health = FMath::Clamp(Health + HealthRegenRate * DeltaTime, 0.f, MaxHealth);
}

void UAttributeComponent::RegenStamina(float DeltaTime)
{
	Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.f, MaxStamina);
}
