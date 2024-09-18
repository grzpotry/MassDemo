// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassMaterialAnimationProcessor.h"

#include "MassCommonFragments.h"
#include "MassDebuggerSubsystem.h"
#include "MassDebugVisualizationComponent.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
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
	bRequiresGameThreadExecution = true; // due to UMassDebuggerSubsystem access
}

void UMassMaterialAnimationProcessor::ConfigureQueries()
{
	EntityQuery.AddSubsystemRequirement<UMassDebuggerSubsystem>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FSimDebugVisFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassEntityHarvesterTag>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassMaterialAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if WITH_EDITORONLY_DATA
	UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.Execute"));
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		UMassDebuggerSubsystem& Debugger = Context.GetMutableSubsystemChecked<UMassDebuggerSubsystem>();
		UMassDebugVisualizationComponent* Visualizer = Debugger.GetVisualizationComponent();
		check(Visualizer);
		TArrayView<UHierarchicalInstancedStaticMeshComponent* const> VisualDataISMCs = Visualizer->GetVisualDataISMCs();
		
		if (VisualDataISMCs.Num() > 0)
		{
			const int32 NumEntities = Context.GetNumEntities();
			const TConstArrayView<FSimDebugVisFragment> DebugVisList = Context.GetFragmentView<FSimDebugVisFragment>();

			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				const FSimDebugVisFragment& VisualComp = DebugVisList[EntityIndex];
				UHierarchicalInstancedStaticMeshComponent* StaticMeshComponent = VisualDataISMCs[VisualComp.VisualType];

				if (!StaticMeshComponent) continue;

				bool bAllTrue = true;
				int prevCustomDataFloats = StaticMeshComponent->NumCustomDataFloats;
				StaticMeshComponent->SetNumCustomDataFloats(4); //expensive, call just once !

				int32 NumInstances = StaticMeshComponent->GetNumInstances();
				FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
				FMassEntityView EntityView(EntityManager, Entity);

				//hardcoded values - just to test solution
				float StartFrame = EntityView.HasTag<FMassHarvesterStateMovingTag>() ? 0 : 23;
				float EndFrame = EntityView.HasTag<FMassHarvesterStateMovingTag>() ? 22 : 58;

				const int32 MeshInstanceIndex = VisualComp.InstanceIndex;
			
				bAllTrue = bAllTrue && StaticMeshComponent->SetCustomDataValue(MeshInstanceIndex, 0, 0.0f); // timeOffset
				bAllTrue = bAllTrue && StaticMeshComponent->SetCustomDataValue(MeshInstanceIndex, 1, 1.0f); // playRate
				bAllTrue = bAllTrue && StaticMeshComponent->SetCustomDataValue(MeshInstanceIndex, 2, StartFrame); // startFrame
				bAllTrue = bAllTrue && StaticMeshComponent->SetCustomDataValue(MeshInstanceIndex, 3, EndFrame); // endFrame
				
				UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.Execute allChangesSucceded: %i instances: %i dataFloats: %i"), bAllTrue, NumInstances, prevCustomDataFloats);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log,
			       TEXT(
				       "UDebugVisLocationProcessor: Trying to update InstanceStaticMeshes while none created. Check your debug visualization setup"
			       ));
		}
	});
#endif
}
