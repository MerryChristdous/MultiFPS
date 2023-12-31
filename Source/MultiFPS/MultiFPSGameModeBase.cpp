// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultiFPSGameModeBase.h"
#include "FPSBaseCharacter.h"
#include "MultiFPSPlayerController.h"


AMultiFPSGameModeBase::AMultiFPSGameModeBase()
{
	//auto PlayerPawnClass = AFPSBaseCharacter::StaticClass();
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(TEXT("/Game/Blueprint/Player/BP_Attacker"));
	if (PlayerPawnClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnClass.Class;
	}
	PlayerControllerClass = AMultiFPSPlayerController::StaticClass();
	
}
