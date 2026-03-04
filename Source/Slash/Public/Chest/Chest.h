#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractibleInterface.h"
#include "Chest.generated.h"

class USphereComponent;

enum class EChestState : uint8
{
	ECS_Closed,
	ECS_Open
};

UCLASS()
class SLASH_API AChest : public AActor, public IInteractibleInterface
{
	GENERATED_BODY()
	
public:	
	AChest();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ChestBaseMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ChestLidMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* CoinsMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Treasure)
	FVector SpawnOffset = FVector(0.f, 0.f, 50.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Treasure)
	FRotator SpawnRotation = FRotator(0.f, 0.f, 0.f);

	EChestState ChestState = EChestState::ECS_Closed;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Interact_Implementation(class ASlashCharacter* Character);

	UFUNCTION()
	void OpenChest();

	UFUNCTION(BlueprintNativeEvent)
	void PlayChestAnimation();

	UFUNCTION(BlueprintCallable)
	void SpawnTreasure();


private:	


	UPROPERTY(EditAnywhere, Category = Treasure)
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;

};
