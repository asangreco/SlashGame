#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes.h"
#include "SlashAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public: 
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	class ASlashCharacter* SlashCharacter;
	UPROPERTY(BlueprintReadWrite, Category = Movement)
	class UCharacterMovementComponent* SlashCharacterMovement;
	UPROPERTY(BlueprintReadWrite, Category = Movement)
	float GroundSpeed;

	UPROPERTY(BlueprintReadWrite, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	ECharacterState CharacterState;

};
