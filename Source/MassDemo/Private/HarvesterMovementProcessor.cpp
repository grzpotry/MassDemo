// Fill out your copyright notice in the Description page of Project Settings.


#include "HarvesterMovementProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
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
	EntityQuery.AddTagRequirement<FMassHarvesterStateMovingTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("Move harvesters"));
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, Context, &EntityManager](FMassExecutionContext& _Context)
{
	const TArrayView<FTransformFragment> TransformsList = _Context.GetMutableFragmentView<FTransformFragment>();
	const TConstArrayView<FHarvesterTargetFragment> TargetsList = _Context.GetFragmentView<FHarvesterTargetFragment>();
	const float WorldDeltaTime = _Context.GetDeltaTimeSeconds();
	const auto EntitiesView  = _Context.GetEntities();

	for (int32 EntityIndex = 0; EntityIndex < _Context.GetNumEntities(); ++EntityIndex)
	{
		FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
		FVector MoveTarget = TargetsList[EntityIndex].WorldPosition;

		FVector CurrentLocation = Transform.GetLocation();
		FVector TargetVector = MoveTarget  - CurrentLocation;

		if (!MoveTarget.IsZero() && TargetVector.Size() > 100.0)
		{
			Transform.SetLocation(CurrentLocation + TargetVector.GetSafeNormal() * 40.0 * WorldDeltaTime);
		}
		else
		{
			//TODO: Except filtering by tag, create separate grid containing only resources !

			if (EntityIndex < EntitiesView.Num())
			{
				FMassEntityHandle ThisEntity = EntitiesView[EntityIndex];

				if (ThisEntity.IsValid())
				{
					_Context.Defer().RemoveTag<FMassHarvesterStateMovingTag>(ThisEntity);
				}
				UE_LOG(LogTemp, Log, TEXT("Move harvester complete"));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("invaid entity"));
			}

		}
	}
});
}
