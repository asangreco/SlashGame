#pragma once


UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two Handed Weapon")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_HitReacting UMETA(DisplayName = "HitReacting"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_Equipping UMETA(DisplayName = "Equipping"),
	EAS_Dodging UMETA(DisplayName = "Dodging"),
};

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_Death1 UMETA(DisplayName = "FallingBackDeath"),
	EDP_Death2 UMETA(DisplayName = "FallingForwardDeath"),
	EDP_Death3 UMETA(DisplayName = "FlyingBackwardsDeath"),
	EDP_Death4 UMETA(DisplayName = "StandingDeathLeft"),
	EDP_Death5 UMETA(DisplayName = "StandingDeathRight"),
	EDP_Death6 UMETA(DisplayName = "SlowDeathForward"),
	EDP_Death7 UMETA(DisplayName = "SlowDeathBack"),

	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged")

};