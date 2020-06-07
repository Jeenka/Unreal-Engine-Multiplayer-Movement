// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MultiplayerMovementGameMode.h"
#include "MultiplayerMovementPawn.h"
#include "MultiplayerMovementHud.h"

AMultiplayerMovementGameMode::AMultiplayerMovementGameMode()
{
	DefaultPawnClass = AMultiplayerMovementPawn::StaticClass();
	HUDClass = AMultiplayerMovementHud::StaticClass();
}
