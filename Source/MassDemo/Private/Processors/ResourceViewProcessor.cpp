// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/ResourceViewProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassRepresentationTypes.h"
#include "MassDemo/MassFragments.h"

UResourceViewProcessor::UResourceViewProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UResourceViewProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollectableResourceFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	EntityQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	EntityQuery.AddRequirement<FResourcesWarehouseFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	//
	// EntityQuery.AddTagRequirement<FMassCollectableResourceTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UResourceViewProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& _Context)
	{
		const int32 NumEntities = _Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = _Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FCollectableResourceFragment> ResourceList = _Context.GetFragmentView<FCollectableResourceFragment>();
		const TConstArrayView<FHarvesterFragment> HarvesterList = _Context.GetFragmentView<FHarvesterFragment>();
		const TConstArrayView<FResourcesWarehouseFragment> WarehouseList = _Context.GetFragmentView<FResourcesWarehouseFragment>();
		const UWorld* World = _Context.GetWorld();
		const FVector Offset = FVector(0.0f,0.0f, 200.0f);

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FMassEntityHandle Entity = _Context.GetEntity(EntityIndex);
			FTransform EntityTransform = TransformList[EntityIndex].GetTransform();

			FMassEntityView EntityView(EntityManager, Entity);
			if (EntityView.HasTag<FMassEntityCollectableResourceTag>())
			{
				const float ResourceAmount = ResourceList[EntityIndex].CurrentAmount;
				DrawDebugString(World, EntityTransform.GetLocation() + Offset,
								FString::Printf(TEXT("%.1f"), ResourceAmount), 0, FColor::Blue, 0.1f);
			} //just for debug
			else if (EntityView.HasTag<FMassEntityHarvesterTag>())
			{
				const float ResourceAmount = HarvesterList[EntityIndex].CurrentResources;
				DrawDebugString(World, EntityTransform.GetLocation() + Offset,
								FString::Printf(TEXT("%.1f"), ResourceAmount), 0, FColor::Green, 0.1f);
			}
			else if (EntityView.HasTag<FMassEntityResourcesWarehouseTag>())
			{
				const float ResourceAmount = WarehouseList[EntityIndex].CurrentAmount;
				DrawDebugString(World, EntityTransform.GetLocation() + Offset,
								FString::Printf(TEXT("%.1f"), ResourceAmount), 0, FColor::Yellow, 0.1f);
			}
		}
	});
}
