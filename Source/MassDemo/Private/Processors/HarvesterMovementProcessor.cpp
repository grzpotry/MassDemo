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
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMassHarvesterStateMovingTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	TArray<FMassEntityHandle> EntitiesToSignal;
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, Context, &EntityManager, &EntitiesToSignal](FMassExecutionContext& _Context)
	{
		const TArrayView<FTransformFragment> TransformsList = _Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FHarvesterTargetFragment> TargetsList = _Context.GetMutableFragmentView<FHarvesterTargetFragment>();
		const float WorldDeltaTime = _Context.GetDeltaTimeSeconds();
		const auto EntitiesView  = _Context.GetEntities();
		const int32 NumEntities = _Context.GetNumEntities();

		EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);

		if (NumEntities > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Move harvesters %i"), NumEntities);
		}
			
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
			FVector& MoveTarget = TargetsList[EntityIndex].WorldPosition;

			FVector CurrentLocation = Transform.GetLocation();
			FVector TargetVector = MoveTarget - CurrentLocation;

			if (!MoveTarget.IsZero() && TargetVector.Size() > 100.0)
			{
				Transform.SetLocation(CurrentLocation + TargetVector.GetSafeNormal() * 200.0 * WorldDeltaTime);
			}
			else
			{
				//TODO: Except filtering by tag, create separate grid containing only resources !

				if (EntityIndex < EntitiesView.Num())
				{
					FMassEntityHandle ThisEntity = EntitiesView[EntityIndex];

					if (ThisEntity.IsValid())
					{
						UE_LOG(LogTemp, Log, TEXT("Move harvester complete"));
						_Context.Defer().RemoveTag<FMassHarvesterStateMovingTag>(ThisEntity);
						MoveTarget = FVector::ZeroVector;
						EntitiesToSignal.Add(ThisEntity);
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("invaid entity"));
					}
		
				}

			}
		}
	});

	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::StateTreeActivate, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("Movement completedd for %i entities"), EntitiesToSignal.Num());
	}
	
}
