// Fill out your copyright notice in the Description page of Project Settings.


//#include "HarvesterTargetProcessor.h"
#include "Processors/HarvesterTargetProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassNavigationSubsystem.h"
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
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterTargetFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMassHarvesterStateSearchingTargetTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const FNavigationObstacleHashGrid2D& ObstacleGrid = NavigationSubsystem->GetObstacleGrid();
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, Context, &ObstacleGrid, &EntityManager](FMassExecutionContext& _Context)
	{
		const TConstArrayView<FTransformFragment> TransformList = _Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHarvesterTargetFragment> HarvesterTargetList = _Context.GetMutableFragmentView<FHarvesterTargetFragment>();
		const int32 NumEntities = _Context.GetNumEntities();

		if (NumEntities > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Calculate target harvesters %i"), NumEntities);
		}
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FTransform& Transform = TransformFragment.GetTransform();
			FVector& HarvesterTarget = HarvesterTargetList[EntityIndex].WorldPosition;
		
			auto Distance = HarvesterTarget - Transform.GetLocation();
		
			//we already have some target, and haven't reach it yet
			if (!HarvesterTarget.IsZero() && Distance.SizeSquared() > 10 * 10)
			{
				continue;
			}
		
			//search new target for harvester
			const FVector Extent(QueryExtent, QueryExtent, QueryExtent);
			const FVector QueryOrigin = Transform.TransformPosition(FVector(0.5f*QueryExtent, 0.f, 0.f));
			const FBox QueryBox = FBox(QueryOrigin - 0.5f * Extent, QueryOrigin + 0.5f * Extent);

			DrawDebugBox(GetWorld(), QueryOrigin, Extent, FColor::Green, true);
			
			TArray<FNavigationObstacleHashGrid2D::ItemIDType> NearbyEntities;
			NearbyEntities.Reserve(16);
			ObstacleGrid.Query(QueryBox, NearbyEntities);

			if (NearbyEntities.IsEmpty())
			{
				UE_LOG(LogTemp, Log, TEXT("Can't find any resource"));
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
				UE_LOG(LogTemp, Log, TEXT("Can't find any resource"));
				continue;
			}

			const FMassNavigationObstacleItem NearbyEntity = NearbyEntities[0];
			FMassEntityView EntityView(EntityManager, NearbyEntity.Entity);

			const FTransformFragment& TargetTransform = EntityView.GetFragmentData<FTransformFragment>();
			HarvesterTarget = TargetTransform.GetTransform().GetLocation();

			const FMassEntityHandle Entity = _Context.GetEntity(EntityIndex);
			_Context.Defer().RemoveTag<FMassHarvesterStateSearchingTargetTag>(Entity);
			UE_LOG(LogTemp, Log, TEXT("Setup new target"));
		}
	});
}

void UHarvesterTargetProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	NavigationSubsystem = UWorld::GetSubsystem<UMassNavigationSubsystem>(Owner.GetWorld());
}

