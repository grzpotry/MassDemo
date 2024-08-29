// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/HarvesterMovementProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "MassDemo/MassFragments.h"

UHarvesterMovementProcessor::UHarvesterMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UHarvesterMovementProcessor::ConfigureQueries()
{
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHarvesterConfigSharedFragment>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMassHarvesterStateMovingTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	TArray<FMassEntityHandle> EntitiesToSignal;
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntitiesToSignal](FMassExecutionContext& _Context)
	{
		const FHarvesterConfigSharedFragment& HarvesterConfigSharedFragment = _Context.GetConstSharedFragment<FHarvesterConfigSharedFragment>();
		const float StopDistanceSqr = HarvesterConfigSharedFragment.TargetStopDistance * HarvesterConfigSharedFragment.TargetStopDistance;
		const float MoveSpeed = HarvesterConfigSharedFragment.MoveSpeed;

		const TArrayView<FTransformFragment> TransformsList = _Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FHarvesterTargetFragment> TargetsList = _Context.GetMutableFragmentView<FHarvesterTargetFragment>();
		const float WorldDeltaTime = _Context.GetDeltaTimeSeconds();
		const auto EntitiesView  = _Context.GetEntities();
		const int32 NumEntities = _Context.GetNumEntities();

		EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);
			
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
			FVector& MoveTarget = TargetsList[EntityIndex].WorldPosition;

			FVector CurrentLocation = Transform.GetLocation();
			FVector TargetVector = MoveTarget - CurrentLocation;
			
			if (!MoveTarget.IsZero() && TargetVector.SizeSquared() > StopDistanceSqr)
			{
				Transform.SetLocation(CurrentLocation + TargetVector.GetSafeNormal() * MoveSpeed * WorldDeltaTime);
			}
			else
			{
				//TODO: Except filtering by tag, create separate grid containing only resources !
				if (EntityIndex < EntitiesView.Num())
				{
					FMassEntityHandle Entity = EntitiesView[EntityIndex];

					if (Entity.IsValid())
					{
						UE_LOG(LogTemp, Log, TEXT("Move harvester complete"));
						_Context.Defer().RemoveTag<FMassHarvesterStateMovingTag>(Entity);
						MoveTarget = FVector::ZeroVector; //clear target on reach, so it won't be relevant anymore
						EntitiesToSignal.Add(Entity);
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("invalid entity"));
					}
				}

			}
		}
	});

	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("Movement completedd for %i entities"), EntitiesToSignal.Num());
	}
	
}
