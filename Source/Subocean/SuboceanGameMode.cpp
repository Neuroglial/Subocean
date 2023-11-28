// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuboceanGameMode.h"
#include "SuboceanCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASuboceanGameMode::ASuboceanGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
