// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCar.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "CarMovementComponent.h"
#include "CarMovementReplicator.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABaseCar::ABaseCar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = false;

	MovementComponent = CreateDefaultSubobject<UCarMovementComponent>(TEXT("Movement Component"));
	MovementReplicatorComponent = CreateDefaultSubobject<UCarMovementReplicator>(TEXT("Movement Replicator"));
}

// Text selection for displaying actor role in game
FString GetEnumText(ENetRole ActorRole)
{
	switch (ActorRole)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called when the game starts or when spawned
void ABaseCar::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

// Called every frame
void ABaseCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void ABaseCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCar::MoveRight);
}

void ABaseCar::MoveForward(float AxisValue)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SetThrottle(AxisValue);
}

void ABaseCar::MoveRight(float AxisValue)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SetSteeringThrow(AxisValue);
}

