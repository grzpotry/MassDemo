// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/SimpleMovementRandomProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassDemo/MassFragments.h"

USimpleMovementRandomProcessor::USimpleMovementRandomProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void USimpleMovementRandomProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FSimpleMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void USimpleMovementRandomProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& InContext)
	{
		const TArrayView<FTransformFragment> TransformsList = InContext.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FSimpleMovementFragment> MovementsList = InContext.GetMutableFragmentView<FSimpleMovementFragment>();
		const float WorldDeltaTime = InContext.GetDeltaTimeSeconds();

		for (int32 EntityIndex = 0; EntityIndex < InContext.GetNumEntities(); ++EntityIndex)
		{
			FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
			FVector& MoveTarget = MovementsList[EntityIndex].Target;

			FVector CurrentLocation = Transform.GetLocation();
			FVector TargetVector = MoveTarget  - CurrentLocation;

			if (MoveTarget.IsZero() || TargetVector.Size() <= 20.0)
			{
				MoveTarget = FVector(CurrentLocation.X + FMath::RandRange(-1.0, 1.0) * 1000.0, CurrentLocation.Y + FMath::RandRange(-1.0, 1.0) * 1000.0,CurrentLocation.Z);
			}
			else
			{
				Transform.SetLocation(CurrentLocation + TargetVector.GetSafeNormal() * 400.0 * WorldDeltaTime);
			}
		}
	});
}
