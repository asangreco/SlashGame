
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlashOverlay.generated.h"


UCLASS()
class SLASH_API USlashOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void SetHealthBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);

	void SetGoldCountText(int32 GoldCount);
	void SetSoulsCountText(int32 SoulsCount);


	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaProgressBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldCountText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SoulsCountText;
};
