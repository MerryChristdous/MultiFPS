// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiFPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIFPS_API AMultiFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake);

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void CreatePlayerUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void DoCrossHairRecoil();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void UpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void UpdateHealthUI(float InHealth, float InHealthPercent, float InDamage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void DeathMatchDeath(AController* InstigatedBy);
	
	
};
