// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonMPGameMode.h"
#include "ThirdPersonMPHUD.h"
#include "ThirdPersonMPCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
AThirdPersonMPGameMode::AThirdPersonMPGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	//PlayerStateClass = AMyPlayerState::StaticClass();

	// use our custom HUD class
	HUDClass = AThirdPersonMPHUD::StaticClass();
	FName url = "127.0.0.1";

	//FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	//if (Context != nullptr)
	//{
		//UGameplayStatics::OpenLevel(this, url);
//	}

	//openLevel
}
