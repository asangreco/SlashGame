#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "interfaces/InteractibleInterface.h"
#include "Item.generated.h"

class USphereComponent;

enum class EItemState : uint8
{
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class SLASH_API AItem : public AActor, public IInteractibleInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(class ASlashCharacter* Character);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float Amplitude = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float TimeConstant = 5.f;


	UFUNCTION(BlueprintPure)
	float TransformSine();

	UFUNCTION(BlueprintPure)
	float TransformCosine();

	virtual void SpawnPickupSystem();
	virtual void SpawnPickupSound();
	virtual void SpawnPickupSystem(AActor* OtherActor);

	// Creating a template function to calculate average, can take many data types T
	template<typename T>
	T Avg(T First, T Second);

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USphereComponent* SphereComponent;


	EItemState ItemState = EItemState::EIS_Hovering;

	UPROPERTY(EditAnywhere, Category = Effects)
	class UNiagaraComponent* ItemEffect;

private: 

	//The meta specifier allows the private variable to be exposed to the Event Graph in blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	class UNiagaraSystem* PickupEffect;

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	class USoundBase* PickupSound;
};

//Definition of Avg Template function
template<typename T>
inline T AItem::Avg(T First, T Second) //Inline makes it so that the function body is ran right when the flow of execution reaches where its called, instead of diverting the flow
{
	return (First + Second) / 2;
}

