// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CarMovementComponent.generated.h"

USTRUCT()
struct FCarMove
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(SteeringThrow) <= 1;
	}
};

/*
* Project specific car movement component to separate movement logic from car actor
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERMOVEMENT_API UCarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	// Mass of the car in kg
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// Force applied to car when the throttle is fully down
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// Minimum radius of car turning circle at full lock
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	// Drag force of air resistance (MaxDrivingForce / MaxSpeed^2, max speed is maximum speed of vehicle in meters per second)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	// Resistance value of rolling on surface
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	float Throttle;

	float SteeringThrow;

	FVector Velocity;

	FCarMove LastMove;

public:	
	// Sets default values for this component's properties
	UCarMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

	void SetThrottle(float InAxisValue) { Throttle = InAxisValue; }

	void SetSteeringThrow(float InAxisValue) { SteeringThrow = InAxisValue; }

	void SimulateMove(const FCarMove& Move);

	FVector GetVelocity() { return Velocity; }

	FCarMove GetLastMove() { return LastMove; }

private:

	FCarMove CreateMove(float DeltaTime);

	FVector GetAirResistance() const;

	FVector GetRollingResistance() const;

	void ApplyRotation(float DeltaTime, float InSteeringThrow);

	void UpdateLocationFromVelocity(float DeltaTime);

};
