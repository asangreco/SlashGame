#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Main interface
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void CycleLockOnTarget(bool bCycleRight);

	UFUNCTION(BlueprintPure, Category = "Lock On")
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintPure, Category = "Lock On")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "Lock On")
	FVector GetCurrentTargetLocation() const;

	// Events for Blueprint to handle
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, AActor*, NewTarget);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockOnBroken);

	UPROPERTY(BlueprintAssignable, Category = "Lock On|Events")
	FOnLockOnTargetChanged OnLockOnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lock On|Events")
	FOnLockOnBroken OnLockOnBroken;

protected:
	virtual void BeginPlay() override;

private:	
	// Target management
	UPROPERTY()
	AActor* CurrentTarget;

	UPROPERTY()
	APlayerController* OwningController;

	UPROPERTY()
	class USpringArmComponent* CameraBoom;

	bool bIsLockedOn;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	float MaxLockOnDistance = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	float BreakLockDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	float MaxLockOnAngle = 60.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	float MaxLockOnAngleWhenLocked = 120.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	float BreakLockBehindAngle = 150.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Detection")
	FName TargetTag = FName("Enemy");

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	float CameraInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	float LockedCameraDistance = 250.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	float DefaultCameraDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	FVector TargetSocketOffset = FVector(0.f, 0.f, 100.f);

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	float MinCameraPitch = -30.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Camera")
	float MaxCameraPitch = 30.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Widget")
	TSubclassOf<class UUserWidget> LockOnWidgetClass;

	UPROPERTY()
	class UWidgetComponent* CurrentLockOnWidget;

	UPROPERTY(EditAnywhere, Category = "Lock On|Widget")
	FVector WidgetOffset = FVector(0.f, 0.f, 100.f);

	UPROPERTY(EditAnywhere, Category = "Lock On|Movement")
	float CharacterRotationSpeed = 10.f;

	AActor* FindBestLockOnTarget() const;
	TArray<AActor*> GetPotentialTargets() const;
	bool IsValidTarget(AActor* Target) const;
	bool IsValidTarget(AActor* Target, bool bIsCurrentTarget) const;
	float CalculateTargetScore(AActor* Target) const;
	void UpdateLockOn(float DeltaTime);
	void BreakLockOn();
	void ApplyLockOn();
	void UpdateCameraRotation(float DeltaTime);
	void UpdateCharacterRotation(float DeltaTime);
	bool HasLineOfSight(AActor* Target) const;
	FVector GetTargetLockLocation(AActor* Target) const;
	void CreateLockOnWidget();
	void RemoveLockOnWidget();
};
