// Fill out your copyright notice in the Description page of Project Settings.


#include "HarvesterMovementProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassDemo/MassFragments.h"

UHarvesterMovementProcessor::UHarvesterMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UHarvesterMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("Move harvesters"));
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& InContext)
{
	const TArrayView<FTransformFragment> TransformsList = InContext.GetMutableFragmentView<FTransformFragment>();
	const TConstArrayView<FHarvesterTargetFragment> TargetsList = InContext.GetFragmentView<FHarvesterTargetFragment>();
	const float WorldDeltaTime = InContext.GetDeltaTimeSeconds();

	for (int32 EntityIndex = 0; EntityIndex < InContext.GetNumEntities(); ++EntityIndex)
	{
		FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
		FVector MoveTarget = TargetsList[EntityIndex].WorldPosition;

		FVector CurrentLocation = Transform.GetLocation();
		FVector TargetVector = MoveTarget  - CurrentLocation;

		if (!MoveTarget.IsZero() && TargetVector.Size() > 100.0)
		{
			Transform.SetLocation(CurrentLocation + TargetVector.GetSafeNormal() * 40.0 * WorldDeltaTime);
		}
	}
});
}
