#include "Items/Weapons/TwoHandedWeapon.h"

ATwoHandedWeapon::ATwoHandedWeapon()
{
	SetBaseDamage(30.f);
	SetDamage(GetBaseDamage());
	SetIsTwoHanded(true);
}

void ATwoHandedWeapon::BeginPlay()
{
	Super::BeginPlay();
}

