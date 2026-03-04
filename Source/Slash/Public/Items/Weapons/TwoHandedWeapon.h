#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/Weapon.h"
#include "TwoHandedWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API ATwoHandedWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	ATwoHandedWeapon();

protected:
	virtual void BeginPlay() override;

private:

};
