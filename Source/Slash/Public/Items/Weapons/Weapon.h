
#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:

	AWeapon();

	void Equip(USceneComponent* InParent, FName InSocketName, FName WeaponSocketName, AActor* NewOwner, APawn* NewInstigatorS);

	void DeactivateEmbers();

	void DisableSphereCollision();

	void PlayEquipSound();

	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName, const FName& WeaponSocketName);

	TArray<AActor*> IgnoredActors;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float BaseDamage = 20.f;

	float Damage;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bIsTwoHanded = false;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool ActorIsSameType(AActor* OtherActor);

	void ExecuteGetHit(FHitResult& BoxHit);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

	void BoxTrace(FHitResult& BoxHit);

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f, 5.f, 5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;


public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }

	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE void SetDamage(float InDamage) { Damage = InDamage; }
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }
	FORCEINLINE void SetBaseDamage(float InBaseDamage) { BaseDamage = InBaseDamage; }
	FORCEINLINE bool IsTwoHanded() const { return bIsTwoHanded; }
	FORCEINLINE void SetIsTwoHanded(bool bInIsTwoHanded) { bIsTwoHanded = bInIsTwoHanded; }

};
