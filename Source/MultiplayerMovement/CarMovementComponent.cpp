// Fill out your copyright notice in the Description page of Project Settings.


#include "CarMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UCarMovementComponent::UCarMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCarMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UCarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// We are the player or server which in control of the pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

void UCarMovementComponent::SimulateMove(const FCarMove& Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();
	FVector Acceleration = Force / Mass;
	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FCarMove UCarMovementComponent::CreateMove(float DeltaTime)
{
	FCarMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;
	return Move;
}

FVector UCarMovementComponent::GetAirResistance() const
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UCarMovementComponent::GetRollingResistance() const
{
	float GravityAcceleration = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * GravityAcceleration;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void UCarMovementComponent::ApplyRotation(float DeltaTime, float InSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
	FQuat RotationDetlta(GetOwner()->GetActorUpVector(), RotationAngle);
	Velocity = RotationDetlta.RotateVector(Velocity);
	GetOwner()->AddActorWorldRotation(RotationDetlta);
}

void UCarMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector TranslationInSpace = Velocity * 100 * DeltaTime;

	FHitResult SweepHit;
	GetOwner()->AddActorWorldOffset(TranslationInSpace, true, &SweepHit);

	if (SweepHit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

