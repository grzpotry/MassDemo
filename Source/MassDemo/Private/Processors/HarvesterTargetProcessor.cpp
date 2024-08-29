// Fill out your copyright notice in the Description page of Project Settings.


//#include "HarvesterTargetProcessor.h"
#include "Processors/HarvesterTargetProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassNavigationSubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "Algo/RandomShuffle.h"
#include "MassDemo/MassFragments.h"

UHarvesterTargetProcessor::UHarvesterTargetProcessor() : EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Tasks);
}

void UHarvesterTargetProcessor::ConfigureQueries()
{
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHarvesterConfigSharedFragment>(EMassFragmentPresence::All);
	
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMassHarvesterStateSearchingTargetTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	TArray<FMassEntityHandle> EntitiesToSignal;
	const FNavigationObstacleHashGrid2D& ObstacleGrid = NavigationSubsystem->GetObstacleGrid();
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &ObstacleGrid, &EntityManager, &EntitiesToSignal](FMassExecutionContext& _Context)
	{
		const TConstArrayView<FTransformFragment> TransformList = _Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHarvesterTargetFragment> HarvesterTargetList = _Context.GetMutableFragmentView<FHarvesterTargetFragment>();
		const int32 NumEntities = _Context.GetNumEntities();
		const FHarvesterConfigSharedFragment& HarvesterConfigSharedFragment = _Context.GetConstSharedFragment<FHarvesterConfigSharedFragment>();
		const float StopDistanceSqr = HarvesterConfigSharedFragment.TargetStopDistance * HarvesterConfigSharedFragment.TargetStopDistance;;
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);
			
			const FTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FTransform& Transform = TransformFragment.GetTransform();
			FVector& HarvesterTarget = HarvesterTargetList[EntityIndex].WorldPosition;

			FVector EntityLocation = Transform.GetLocation();
			FVector Distance = HarvesterTarget - EntityLocation;
			
			//we already have some target, and haven't reach it yet
			if (!HarvesterTarget.IsZero() && Distance.SizeSquared() > StopDistanceSqr)
			{
				continue;
			}
		
			//search new target for harvester
			const FVector Extent(QueryExtent, QueryExtent, QueryExtent);
			const FBox QueryBox = FBox::BuildAABB(EntityLocation, Extent);
			
			TArray<FNavigationObstacleHashGrid2D::ItemIDType> NearbyEntities;
			NearbyEntities.Reserve(16);
			ObstacleGrid.Query(QueryBox, NearbyEntities);

			if (NearbyEntities.IsEmpty())
			{
				OnResourceSearchFailed(EntityLocation, Extent);
				continue;
			}

			//pick random
			Algo::RandomShuffle(NearbyEntities);

			for (int i = NearbyEntities.Num() - 1; i >= 0; i--)
			{
				// This can happen if we remove entities in the system.
				if (!EntityManager.IsEntityValid(NearbyEntities[i].Entity))
				{
					NearbyEntities.RemoveAt(i);
				}

				//TODO: Except filtering by tag, create separate grid containing only resources !
				FMassEntityView EntityView(EntityManager, NearbyEntities[i].Entity);
				if (!EntityView.HasTag<FMassCollectableResourceTag>())
				{
					NearbyEntities.RemoveAt(i);
				}
			}

			if (NearbyEntities.IsEmpty())
			{
				OnResourceSearchFailed(EntityLocation, Extent);
				continue;
			}

			const FMassNavigationObstacleItem NearbyEntity = NearbyEntities[0];
			FMassEntityView EntityView(EntityManager, NearbyEntity.Entity);

			const FTransformFragment& TargetTransform = EntityView.GetFragmentData<FTransformFragment>();
			HarvesterTarget = TargetTransform.GetTransform().GetLocation();

			const FMassEntityHandle Entity = _Context.GetEntity(EntityIndex);
			_Context.Defer().RemoveTag<FMassHarvesterStateSearchingTargetTag>(Entity);

			EntitiesToSignal.Add(Entity);
		}
	});
	
	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("Setup new target for %i entities"), EntitiesToSignal.Num());
	}
}

void UHarvesterTargetProcessor::OnResourceSearchFailed(const FVector& QueryOrigin, const FVector& Extent) const
{
	DrawDebugBox(GetWorld(), QueryOrigin, Extent, FColor::Yellow, true, 10);
	UE_LOG(LogTemp, Log, TEXT("Can't find any resource1234"));
}

void UHarvesterTargetProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	NavigationSubsystem = UWorld::GetSubsystem<UMassNavigationSubsystem>(Owner.GetWorld());
}

