// Fill out your copyright notice in the Description page of Project Settings.


//#include "HarvesterTargetProcessor.h"
#include "MassDemo/HarvesterTargetProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassNavigationSubsystem.h"
#include "MassDemo/MassFragments.h"

UHarvesterTargetProcessor::UHarvesterTargetProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UHarvesterTargetProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UHarvesterTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("Process harvesters"));
	const FNavigationObstacleHashGrid2D& ObstacleGrid = NavigationSubsystem->GetObstacleGrid();
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &ObstacleGrid, &EntityManager](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHarvesterTargetFragment> HarvesterTargetList = Context.GetMutableFragmentView<FHarvesterTargetFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		
		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FTransformFragment& TransformFragment = TransformList[i];
			const FTransform& Transform = TransformFragment.GetTransform();
			FVector& HarvesterTarget = HarvesterTargetList[i].WorldPosition;
		
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
		
			for (const FNavigationObstacleHashGrid2D::ItemIDType NearbyEntity : NearbyEntities)
			{
				// This can happen if we remove entities in the system.
				if (!EntityManager.IsEntityValid(NearbyEntity.Entity))
				{
					continue;
				}
		
				//TODO: Except filtering by tag, create separate grid containing only resources !
				FMassEntityView EntityView(EntityManager, NearbyEntity.Entity);
				if (!EntityView.HasTag<FMassCollectableResourceTag>())
				{
					continue;
				}
		
				const FTransformFragment& TargetTransform = EntityView.GetFragmentData<FTransformFragment>();
				HarvesterTarget = TargetTransform.GetTransform().GetLocation();
				UE_LOG(LogTemp, Log, TEXT("Setup new target"));
			}
		}
	});
	
	//Super::Execute(EntityManager, Context);
}

void UHarvesterTargetProcessor::Initialize(UObject& Owner)
{
	UE_LOG(LogTemp, Log, TEXT("Initialize"));
	Super::Initialize(Owner);
	
	NavigationSubsystem = UWorld::GetSubsystem<UMassNavigationSubsystem>(Owner.GetWorld());
}

