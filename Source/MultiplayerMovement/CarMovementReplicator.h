// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarMovementComponent.h"

#include "CarMovementReplicator.generated.h"

struct FCarMove;

USTRUCT()
struct FCarState
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FCarMove LastMove;
};

struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FVector InterpolateLocation(const float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FVector InterpolateDerivative(const float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

/*
* Replication component to handle car movement replication logic, depends on "CarMovementComponent"
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERMOVEMENT_API UCarMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCarMovementReplicator();

private:

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FCarState ServerState;

	UPROPERTY(VisibleAnywhere)
	UCarMovementComponent* MovementComponent;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	TArray<FCarMove> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;

	FVector ClientStartVelocity;

	float ClientSimulatedTime;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* InRoot) { MeshOffsetRoot = InRoot; }

private:

	void ClientTick(float DeltaTime);

	void InterpolateLocation(const FHermiteCubicSpline &Spline, const float LerpRatio);

	void InterpolateVelocity(const FHermiteCubicSpline &Spline, const float LerpRatio);

	void InterpolateRotation(const float LerpRatio);

	// Spline to along which to correct client delay
	FHermiteCubicSpline CreateSpline();

	float GetVelocityToDerivative();

	// Save moves by delta time in array to compare it with server state later
	void ClearAcknowlegedMoves(const FCarMove& LastMove);	

	void UpdateServerState(const FCarMove& Move);

	UFUNCTION()
	void OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FCarMove Move);

};
