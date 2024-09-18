// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassMaterialAnimationProcessor.h"

#include "MassCommonFragments.h"
#include "MassDebuggerSubsystem.h"
#include "MassDebugVisualizationComponent.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MassDemo/MassFragments.h"
#include "Materials/MaterialInstanceConstant.h"

UMassMaterialAnimationProcessor::UMassMaterialAnimationProcessor()
{
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorldToMass);

	ObservedType = FHarvesterFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;

	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true; // todo: still necessary ?
}

void UMassMaterialAnimationProcessor::ConfigureQueries()
{
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.AddTagRequirement<FMassEntityHarvesterTag>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassMaterialAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor1.Execute"));

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetMutableSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);

		const FMassInstancedStaticMeshInfoArrayView ISMInfosView = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		
		const int32 NumEntities = Context.GetNumEntities();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			FMassEntityView EntityView(EntityManager, Entity);
			
			FMassRepresentationFragment& Representation = EntityView.GetFragmentData<FMassRepresentationFragment>();
			const int32 StaticMeshViewIndex = Representation.StaticMeshDescHandle.ToIndex();

			if (!ISMInfosView.IsValidIndex(StaticMeshViewIndex))
			{
				continue;
			}
			
			FMassInstancedStaticMeshInfo& ISMInfo = ISMInfosView[StaticMeshViewIndex];

			if (!ISMInfo.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("UMassMaterialAnimationProcessor.InvalidEntityMesh"));
				continue;
			}

			//hardcoded values - just to test solution
			const float StartFrame = EntityView.HasTag<FMassHarvesterStateMovingTag>() ? 0 : 23;
			const float EndFrame = EntityView.HasTag<FMassHarvesterStateMovingTag>() ? 22 : 58;
			constexpr float TimeOffset = 0.0f;
			constexpr float PlayRate = 1.0f;

			TArray CustomFloats = {TimeOffset, PlayRate, StartFrame, EndFrame};
			ISMInfo.AddBatchedCustomDataFloats(CustomFloats, 0);

			UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.Execute"));
		}
	});
}
