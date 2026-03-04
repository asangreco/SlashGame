#include "Components/LockOnComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Characters/BaseCharacter.h"
#include "Enemy/Enemy.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsLockedOn = false;
	CurrentTarget = nullptr;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache references
	OwningController = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	
	if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
	{
		CameraBoom = OwningCharacter->FindComponentByClass<USpringArmComponent>();
	}
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockedOn)
	{
		UpdateLockOn(DeltaTime);
	}
}

void ULockOnComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		BreakLockOn();
	}
	else
	{
		AActor* Target = FindBestLockOnTarget();
		if (Target)
		{
			CurrentTarget = Target;
			bIsLockedOn = true;
			ApplyLockOn();
			CreateLockOnWidget();
			OnLockOnTargetChanged.Broadcast(CurrentTarget);
		}
	}
}

void ULockOnComponent::CycleLockOnTarget(bool bCycleRight)
{
	if (!bIsLockedOn) return;

	TArray<AActor*> PotentialTargets = GetPotentialTargets();
	if (PotentialTargets.Num() <= 1) return;

	// Sort by angle from current target
	FVector CurrentDirection = CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation();
	CurrentDirection.Z = 0;
	CurrentDirection.Normalize();

	PotentialTargets.Sort([&](const AActor& A, const AActor& B) {
		FVector DirA = A.GetActorLocation() - GetOwner()->GetActorLocation();
		DirA.Z = 0;
		DirA.Normalize();

		FVector DirB = B.GetActorLocation() - GetOwner()->GetActorLocation();
		DirB.Z = 0;
		DirB.Normalize();

		float AngleA = FMath::Acos(FVector::DotProduct(CurrentDirection, DirA));
		float AngleB = FMath::Acos(FVector::DotProduct(CurrentDirection, DirB));

		// Use cross product to determine left/right
		float CrossA = FVector::CrossProduct(CurrentDirection, DirA).Z;
		float CrossB = FVector::CrossProduct(CurrentDirection, DirB).Z;

		if (bCycleRight)
		{
			return (CrossA > 0 && CrossB > 0) ? AngleA < AngleB : CrossA > CrossB;
		}
		else
		{
			return (CrossA < 0 && CrossB < 0) ? AngleA < AngleB : CrossA < CrossB;
		}
	});

	// Find next target that isn't current
	for (AActor* Target : PotentialTargets)
	{
		if (Target != CurrentTarget)
		{
			RemoveLockOnWidget();
			CurrentTarget = Target;
			CreateLockOnWidget();
			OnLockOnTargetChanged.Broadcast(CurrentTarget);
			return;
		}
	}
}

FVector ULockOnComponent::GetCurrentTargetLocation() const
{
	if (CurrentTarget)
	{
		return GetTargetLockLocation(CurrentTarget);
	}
	return FVector::ZeroVector;
}

AActor* ULockOnComponent::FindBestLockOnTarget() const
{
	TArray<AActor*> PotentialTargets = GetPotentialTargets();
	
	AActor* BestTarget = nullptr;
	float BestScore = FLT_MAX;

	for (AActor* Target : PotentialTargets)
	{
		if (!IsValidTarget(Target)) continue;

		float Score = CalculateTargetScore(Target);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

TArray<AActor*> ULockOnComponent::GetPotentialTargets() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);
	
	// Filter by distance
	FoundActors.RemoveAll([this](AActor* Actor) {
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
		return Distance > MaxLockOnDistance || Actor == GetOwner();
	});

	return FoundActors;
}

bool ULockOnComponent::IsValidTarget(AActor* Target) const
{
	return IsValidTarget(Target, false);
}

bool ULockOnComponent::IsValidTarget(AActor* Target, bool bIsCurrentTarget) const
{
	if (!Target || Target == GetOwner()) return false;

	// Check if target is dead (for enemies - use Enemy's IsDead which checks state)
	if (AEnemy* Enemy = Cast<AEnemy>(Target))
	{
		if (Enemy->IsDead()) return false;
	}
	// Fallback to IsAlive for other BaseCharacter types
	else if (ABaseCharacter* BaseChar = Cast<ABaseCharacter>(Target))
	{
		if (!BaseChar->IsAlive()) return false;
	}

	// Check distance
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MaxLockOnDistance) return false;

	// Check angle from camera - use more lenient angle if already locked on
	FVector ToTarget = Target->GetActorLocation() - GetOwner()->GetActorLocation();
	ToTarget.Normalize();
	
	if (OwningController)
	{
		FVector CameraForward = OwningController->GetControlRotation().Vector();
		float Angle = FMath::Acos(FVector::DotProduct(CameraForward, ToTarget)) * (180.f / PI);
		
		// Use different angle thresholds for acquiring vs maintaining lock
		float AngleThreshold = bIsCurrentTarget ? MaxLockOnAngleWhenLocked : MaxLockOnAngle;
		
		if (Angle > AngleThreshold) return false;
	}

	// Check line of sight - more lenient for current target
	if (!bIsCurrentTarget || Distance < BreakLockDistance * 0.5f)
	{
		return HasLineOfSight(Target);
	}

	return true;
}

float ULockOnComponent::CalculateTargetScore(AActor* Target) const
{
	if (!OwningController) return FLT_MAX;

	// Project to screen space
	FVector2D ScreenPosition;
	if (OwningController->ProjectWorldLocationToScreen(Target->GetActorLocation(), ScreenPosition))
	{
		int32 ViewportX, ViewportY;
		OwningController->GetViewportSize(ViewportX, ViewportY);

		// Distance from screen center (prefer targets near crosshair)
		FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);
		float ScreenDistance = FVector2D::Distance(ScreenPosition, ScreenCenter);

		// World distance
		float WorldDistance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

		// Weighted score (screen position more important than distance)
		return ScreenDistance + (WorldDistance * 0.1f);
	}

	return FLT_MAX;
}

void ULockOnComponent::UpdateLockOn(float DeltaTime)
{
	if (!CurrentTarget)
	{
		BreakLockOn();
		return;
	}

	// Check if target is still valid (includes alive check) - use lenient check for current target
	if (!IsValidTarget(CurrentTarget, true))
	{
		BreakLockOn();
		return;
	}

	// Check if target is too far
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > BreakLockDistance)
	{
		BreakLockOn();
		return;
	}

	// Check if target is too far behind (prevents camera from snapping backwards violently)
	FVector ToTarget = (CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	FVector OwnerForward = GetOwner()->GetActorForwardVector();
	float DotProduct = FVector::DotProduct(OwnerForward, ToTarget);
	float AngleDegrees = FMath::Acos(DotProduct) * (180.f / PI);
	
	// If target is more than BreakLockBehindAngle degrees behind, break lock
	if (AngleDegrees > BreakLockBehindAngle)
	{
		BreakLockOn();
		return;
	}

	UpdateCameraRotation(DeltaTime);
	UpdateCharacterRotation(DeltaTime);
}

void ULockOnComponent::UpdateCameraRotation(float DeltaTime)
{
	if (!OwningController || !CurrentTarget) return;

	FVector TargetLocation = GetTargetLockLocation(CurrentTarget);
	FVector CameraLocation = OwningController->PlayerCameraManager->GetCameraLocation();
	
	FRotator LookAtRotation = (TargetLocation - CameraLocation).Rotation();
	FRotator CurrentRotation = OwningController->GetControlRotation();
	
	// Clamp pitch to prevent extreme top-down or bottom-up views
	LookAtRotation.Pitch = FMath::Clamp(LookAtRotation.Pitch, MinCameraPitch, MaxCameraPitch);
	
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, CameraInterpSpeed);
	OwningController->SetControlRotation(NewRotation);
}

void ULockOnComponent::UpdateCharacterRotation(float DeltaTime)
{
	if (!CurrentTarget) return;

	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	if (!OwningCharacter) return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector OwnerLocation = OwningCharacter->GetActorLocation();
	
	FRotator LookAtRotation = (TargetLocation - OwnerLocation).Rotation();
	FRotator CurrentRotation = OwningCharacter->GetActorRotation();
	
	// Only rotate yaw
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll),
		DeltaTime,
		CharacterRotationSpeed
	);
	
	OwningCharacter->SetActorRotation(NewRotation);
}

bool ULockOnComponent::HasLineOfSight(AActor* Target) const
{
	if (!Target) return false;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(Target);

	return !GetWorld()->LineTraceSingleByChannel(
		HitResult,
		GetOwner()->GetActorLocation(),
		Target->GetActorLocation(),
		ECC_Visibility,
		Params
	);
}

FVector ULockOnComponent::GetTargetLockLocation(AActor* Target) const
{
	if (!Target) return FVector::ZeroVector;

	// Try to get a specific socket/bone for lock-on (chest/spine area)
	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh();
		if (Mesh && Mesh->DoesSocketExist(FName("LockOnSocket")))
		{
			return Mesh->GetSocketLocation(FName("LockOnSocket"));
		}
	}

	// Default to actor location with offset
	return Target->GetActorLocation() + TargetSocketOffset;
}

void ULockOnComponent::ApplyLockOn()
{
	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	if (!OwningCharacter) return;

	// Keep character rotation behavior - no strafing
	// Note: Character will still rotate via UpdateCharacterRotation in Tick
	
	// Adjust camera distance
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = LockedCameraDistance;
	}
}

void ULockOnComponent::BreakLockOn()
{
	RemoveLockOnWidget();
	
	bIsLockedOn = false;
	CurrentTarget = nullptr;

	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	if (!OwningCharacter) return;

	// Restore camera distance
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = DefaultCameraDistance;
	}

	OnLockOnBroken.Broadcast();
}

void ULockOnComponent::CreateLockOnWidget()
{
	if (!LockOnWidgetClass || !CurrentTarget) return;

	// Remove any existing widget first
	RemoveLockOnWidget();

	// Create widget component and attach to target
	CurrentLockOnWidget = NewObject<UWidgetComponent>(CurrentTarget);
	if (CurrentLockOnWidget)
	{
		CurrentLockOnWidget->RegisterComponent();
		CurrentLockOnWidget->SetWidgetClass(LockOnWidgetClass);
		CurrentLockOnWidget->SetWidgetSpace(EWidgetSpace::Screen);
		CurrentLockOnWidget->SetDrawSize(FVector2D(40.f, 40.f));
		CurrentLockOnWidget->SetRelativeLocation(WidgetOffset);
		CurrentLockOnWidget->SetVisibility(true);
		
		// Attach to target's root
		CurrentLockOnWidget->AttachToComponent(
			CurrentTarget->GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform
		);
	}
}

void ULockOnComponent::RemoveLockOnWidget()
{
	if (CurrentLockOnWidget)
	{
		CurrentLockOnWidget->DestroyComponent();
		CurrentLockOnWidget = nullptr;
	}
}


