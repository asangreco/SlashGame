#include "Chest/Chest.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"
#include "Math/UnrealMathUtility.h"
#include "Items/Treasure.h"

AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = false;

	ChestBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBaseMesh"));
	RootComponent = ChestBaseMesh;
	ChestLidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLidMesh"));
	ChestLidMesh->SetupAttachment(GetRootComponent());
	CoinsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinsMesh"));
	CoinsMesh->SetupAttachment(GetRootComponent());
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(GetRootComponent());

	//Attach Lid to Base at socket BaseHinge
	ChestLidMesh->SetupAttachment(ChestBaseMesh, TEXT("BaseHinge"));


}

void AChest::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AChest::OnSphereOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AChest::OnSphereEndOverlap);


}


void AChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AChest::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(this);
	}
}

void AChest::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(nullptr);
	}
}

void AChest::Interact_Implementation(ASlashCharacter* Character)
{
	if (ChestState == EChestState::ECS_Closed)
	{
		OpenChest();
	}
}

void AChest::OpenChest()
{
	if (ChestState == EChestState::ECS_Closed)
	{
		PlayChestAnimation();
		ChestState = EChestState::ECS_Open;
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AChest::SpawnTreasure()
{
	if (GetWorld() && TreasureClasses.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);

		FVector SpawnLocation = GetActorLocation() + SpawnOffset;
		GetWorld()->SpawnActor<ATreasure>(TreasureClasses[Selection], SpawnLocation, SpawnRotation);
	}
}

void AChest::PlayChestAnimation_Implementation()
{

}
