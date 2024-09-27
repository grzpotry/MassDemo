// Fill out your copyright notice in the Description page of Project Settings.

#include "Processors/Transfer/TransferResourceFromHarvesterToWarehouseProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UTransferResourceFromHarvesterToWarehouseProcessor::UTransferResourceFromHarvesterToWarehouseProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UTransferResourceFromHarvesterToWarehouseProcessor::ConfigureQueries()
{
	Super::ConfigureQueries();
	
	SourceQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddRequirement<FTransferFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddConstSharedRequirement<FHarvesterConfigSharedFragment>(EMassFragmentPresence::All);
	SourceQuery.AddTagRequirement<FMassEntityHarvesterTag>(EMassFragmentPresence::All);
	SourceQuery.AddTagRequirement<FMassHarvesterStateDeliverResourceTag>(EMassFragmentPresence::All);
	SourceQuery.RegisterWithProcessor(*this);
	
	TargetQuery.AddRequirement<FResourcesWarehouseFragment>(EMassFragmentAccess::ReadWrite);
	TargetQuery.AddTagRequirement<FMassEntityResourcesWarehouseTag>(EMassFragmentPresence::All);
	TargetQuery.RegisterWithProcessor(*this);
}

void UTransferResourceFromHarvesterToWarehouseProcessor::Execute(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("UHarvesterDeliverResourceProcessor"));

	ExecuteInternal<FHarvesterFragment, FHarvesterConfigSharedFragment, FResourcesWarehouseFragment, float>(EntityManager, Context,
		/*GetTransferValue*/[](FMassExecutionContext&, const FHarvesterFragment& SourceFragment) -> float
		{
			UE_LOG(LogTemp, Log, TEXT("UHarvesterDeliverResourceProcessor transfer resourses %f"), SourceFragment.CurrentResources);
			return SourceFragment.CurrentResources;
		},
		/*ClampTransferValue*/[&EntityManager](const FHarvesterFragment& SourceFragment, const FHarvesterConfigSharedFragment& SourceSharedFragment, float Value) -> FTransferEntityFloat
		{
			const FMassEntityHandle TargetEntity = SourceFragment.MoveTargetEntityHandle;

			if (!EntityManager.IsEntityValid(TargetEntity))
			{
				return FTransferEntityFloat::Invalid();
			}
			
			const FMassEntityView TargetEntityView(EntityManager, TargetEntity);
			const FResourcesWarehouseFragment* TargetFragment = TargetEntityView.GetFragmentDataPtr<FResourcesWarehouseFragment>();
			
			if (TargetFragment == nullptr)
			{
				//UE_LOG(LogTemp, Error, TEXT("Missing FResourcesWarehouseFragment fragment for entity: %s"), *TargetEntity.DebugGetDescription());
				return FTransferEntityFloat::Invalid();
			}
			
			const float Result = FMath::Min(Value, TargetFragment->MaxCapacity - TargetFragment->CurrentAmount);
			
			return FTransferEntityFloat(TargetEntity, FMath::Min(Result, SourceFragment.CurrentResources));
		},
		/*ProcessSource*/[](FHarvesterFragment& SourceFragment, const FHarvesterConfigSharedFragment& SourceSharedFragment, float Value, FMassEntityHandle SourceEntity, FMassExecutionContext& Context) -> void
		{
			SourceFragment.CurrentResources -= Value;

			if (SourceFragment.CurrentResources < SourceSharedFragment.ResourcesStorageCapacity)
			{
				Context.Defer().RemoveTag<FMassHarvesterIsFullTag>(SourceEntity);
			}
		},
		/*ProcessTarget*/[](FResourcesWarehouseFragment& TargetFragment, float Value) -> void
		{
			TargetFragment.CurrentAmount += Value;
		});
}

void UTransferResourceFromHarvesterToWarehouseProcessor::StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal,
	FMassExecutionContext& Context, FMassEntityHandle EntityHandle)
{
	Context.Defer().RemoveTag<FMassHarvesterStateDeliverResourceTag>(EntityHandle);
	EntitiesToSignal.Add(EntityHandle);
}
