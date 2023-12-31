// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiFPSPlayerController.h"

void AMultiFPSPlayerController::PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake)
{
	if (CameraShake)
	{
		ClientStartCameraShake(CameraShake, 1.f);
	}

}
