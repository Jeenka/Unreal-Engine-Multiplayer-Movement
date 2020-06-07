// Fill out your copyright notice in the Description page of Project Settings.


#include "CarMovementReplicator.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
UCarMovementReplicator::UCarMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UCarMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UCarMovementComponent>();
}

// Called every frame
void UCarMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr)
	{
		return;
	}

	FCarMove LastMove = MovementComponent->GetLastMove();

	// We are the player
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	// We are the server and in control of the pawn
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	// Actor is simulated proxy
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UCarMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCarMovementReplicator, ServerState);
}

void UCarMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (MovementComponent == nullptr)
	{
		return;
	}

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);

	InterpolateRotation(LerpRatio);

}

void UCarMovementReplicator::InterpolateLocation(const FHermiteCubicSpline &Spline, const float LerpRatio)
{ 	
	if (MeshOffsetRoot != nullptr)
	{
		FVector NewLocation = Spline.InterpolateLocation(LerpRatio);

		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
}

void UCarMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline &Spline, const float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / GetVelocityToDerivative();
	MovementComponent->SetVelocity(NewVelocity);
}

void UCarMovementReplicator::InterpolateRotation(const float LerpRatio)
{
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();
		
	if (MeshOffsetRoot != nullptr)
	{
		FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

FHermiteCubicSpline UCarMovementReplicator::CreateSpline()
{
	FHermiteCubicSpline Spline;

	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();

	Spline.StartDerivative = ClientStartVelocity * GetVelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * GetVelocityToDerivative();

	return Spline;
}

float UCarMovementReplicator::GetVelocityToDerivative()
{
	return ClientTimeBetweenLastUpdates * 100;
}

void UCarMovementReplicator::ClearAcknowlegedMoves(const FCarMove& LastMove)
{
	TArray<FCarMove> NewMoves;

	for (const FCarMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UCarMovementReplicator::UpdateServerState(const FCarMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UCarMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UCarMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowlegedMoves(ServerState.LastMove);

	for (const FCarMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UCarMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}

	ClientStartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UCarMovementReplicator::Server_SendMove_Implementation(FCarMove Move)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	ClientSimulatedTime += Move.DeltaTime;

	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UCarMovementReplicator::Server_SendMove_Validate(FCarMove Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool ClientNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;

	if (!ClientNotRunningAhead)
	{
		UE_LOG(LogTemp, Error, TEXT("Client is running too fast"));

		return false;
	}
	
	if (!Move.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Client move is invalid"));

		return false;
	}

	return true;
}
