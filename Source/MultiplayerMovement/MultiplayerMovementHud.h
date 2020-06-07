// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "MultiplayerMovementHud.generated.h"


UCLASS(config = Game)
class AMultiplayerMovementHud : public AHUD
{
	GENERATED_BODY()

public:
	AMultiplayerMovementHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
