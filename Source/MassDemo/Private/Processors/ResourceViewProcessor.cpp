// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/ResourceViewProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
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
	EntityQuery.AddRequirement<FCollectableResourceFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddTagRequirement<FMassCollectableResourceTag>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UResourceViewProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& _Context)
	{
		const int32 NumEntities = _Context.GetNumEntities();
		const TConstArrayView<FTransformFragment> TransformList = _Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FCollectableResourceFragment> ResourceList = _Context.GetFragmentView<FCollectableResourceFragment>();
		const UWorld* World = _Context.GetWorld();
		const FVector Offset = FVector(0.0f,0.0f, 200.0f);

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const float ResourceAmount = ResourceList[EntityIndex].CurrentAmount;
			FTransform EntityTransform = TransformList[EntityIndex].GetTransform();
			DrawDebugString(World, EntityTransform.GetLocation() + Offset, FString::Printf(TEXT("%.1f"), ResourceAmount), 0, FColor::Blue, 0.1f);
		}
	});
}
