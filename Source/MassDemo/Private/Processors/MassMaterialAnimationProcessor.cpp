// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassMaterialAnimationProcessor.h"

#include "MassCommonFragments.h"
#include "MassDebuggerSubsystem.h"
#include "MassDebugVisualizationComponent.h"
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
	UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.Execute"));
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		UMassDebuggerSubsystem& Debugger = Context.GetMutableSubsystemChecked<UMassDebuggerSubsystem>();
		UMassDebugVisualizationComponent* Visualizer = Debugger.GetVisualizationComponent();
		check(Visualizer);
		TArrayView<UHierarchicalInstancedStaticMeshComponent* const> VisualDataISMCs = Visualizer->GetVisualDataISMCs();
		
		if (VisualDataISMCs.Num() > 0)
		{
			const int32 NumEntities = Context.GetNumEntities();
			const TConstArrayView<FTransformFragment> LocationList = Context.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FSimDebugVisFragment> DebugVisList = Context.GetFragmentView<FSimDebugVisFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FSimDebugVisFragment& VisualComp = DebugVisList[i];
				UHierarchicalInstancedStaticMeshComponent* StaticMeshComponent = VisualDataISMCs[VisualComp.VisualType];

				if (!StaticMeshComponent) continue; // Ensure the mesh component is valid

				// Get the material from the static mesh

				// // Assuming the material is applied at index 0 of the static mesh
				// if (StaticMeshComponent->GetMaterial(0))
				// {
				// 	// Create a dynamic material instance for this mesh
				// 	if (UMaterialInstanceDynamic* DynamicMaterialInstance = StaticMeshComponent->
				// 		CreateAndSetMaterialInstanceDynamic(0))
				// 	{
				// 		// Change the scalar parameter called "RoughnessControl"
				// 		DynamicMaterialInstance->SetScalarParameterValue(FName("StartFrame"), 23);
				// 		DynamicMaterialInstance->SetScalarParameterValue(FName("EndFrame"), 58);
				//
				// 		UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.SetupAnimation"));
				// 	}
				// }

				// Access the Material Instance Constant (MIC) applied to the Static Mesh
				UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(StaticMeshComponent->GetMaterial(0));

				if (MaterialInstance)
				{
					// Modify the scalar parameter value on the Material Instance
					FMaterialParameterInfo Param1(FName("StartFrame"));
					FMaterialParameterInfo Param2(FName("EndFrame"));

					MaterialInstance->SetScalarParameterValueEditorOnly(Param1, 23.0f);
					MaterialInstance->SetScalarParameterValueEditorOnly(Param2, 58.0f);

					// Optional: Update the Material Instance to apply the changes
					MaterialInstance->PostEditChange();
				}

				// UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(StaticMeshComponent->GetMaterial(0));
				// if (MaterialInstance)
				// {
				// 	TArray<FMaterialParameterInfo> ParameterInfo;
				// 	TArray<FGuid> ParameterIDs;
				//
				// 	MaterialInstance->GetAllScalarParameterInfo(ParameterInfo, ParameterIDs);
				//
				// 	for (const FMaterialParameterInfo& Info : ParameterInfo)
				// 	{
				// 		UE_LOG(LogTemp, Warning, TEXT("Found Scalar Parameter: %s"), *Info.Name.ToString());
				// 	}
				// }

				UE_LOG(LogTemp, Log, TEXT("UMassMaterialAnimationProcessor.SetupAnimation"));
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
}
