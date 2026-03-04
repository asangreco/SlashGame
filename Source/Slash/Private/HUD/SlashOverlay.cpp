#include "HUD/SlashOverlay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"


void USlashOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetGoldCountText(int32 GoldCount)
{
	if (GoldCountText)
	{
		GoldCountText->SetText(FText::AsNumber(GoldCount));
	}
}

void USlashOverlay::SetSoulsCountText(int32 SoulsCount)
{
	if (SoulsCountText)
	{
		SoulsCountText->SetText(FText::AsNumber(SoulsCount));
	}
}
