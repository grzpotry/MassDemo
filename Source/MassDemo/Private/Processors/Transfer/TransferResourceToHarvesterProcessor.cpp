// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/Transfer/HarvesterMineResourceProcessor.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UHarvesterMineResourceProcessor::UHarvesterMineResourceProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UHarvesterMineResourceProcessor::ConfigureQueries()
{
	Super::ConfigureQueries();
	
	SourceQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddRequirement<FTransferFragment>(EMassFragmentAccess::ReadWrite);
	SourceQuery.AddConstSharedRequirement<FHarvesterConfigSharedFragment>(EMassFragmentPresence::All);
	SourceQuery.AddTagRequirement<FMassEntityHarvesterTag>(EMassFragmentPresence::All);
	SourceQuery.AddTagRequirement<FMassHarvesterStateMiningResourceTag>(EMassFragmentPresence::All);
	SourceQuery.RegisterWithProcessor(*this);
	
	TargetQuery.AddRequirement<FCollectableResourceFragment>(EMassFragmentAccess::ReadWrite);
	TargetQuery.AddTagRequirement<FMassEntityCollectableResourceTag>(EMassFragmentPresence::All);
	TargetQuery.RegisterWithProcessor(*this);
}

void UHarvesterMineResourceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Log, TEXT("UHarvesterMineResourceProcessor"));

	ExecuteInternal<FHarvesterFragment, FCollectableResourceFragment, float>(EntityManager, Context,
		/*GetTransferValue*/[](FMassExecutionContext& _Context) -> float
		{
			const FHarvesterConfigSharedFragment& HarvesterConfigSharedFragment = _Context.GetConstSharedFragment<FHarvesterConfigSharedFragment>();
			return HarvesterConfigSharedFragment.MiningResourceSpeed;
		},
		/*ClampTransferValue*/[&EntityManager](const FHarvesterFragment& SourceFragment, float Value) -> FTransferEntityFloat
		{
			const FMassEntityHandle TargetEntity = SourceFragment.MoveTargetEntityHandle;

			if (!TargetEntity.IsValid())
			{
				return FTransferEntityFloat::Invalid();
			}

			const float Capacity = SourceFragment.ResourcesStorageCapacity;
			const FMassEntityView TargetEntityView(EntityManager, TargetEntity);
			const FCollectableResourceFragment& TargetFragment = TargetEntityView.GetFragmentData<FCollectableResourceFragment>();
			const float Result = FMath::Min(Value, TargetFragment.CurrentAmount);
			
			return FTransferEntityFloat(TargetEntity, FMath::Min(Result, Capacity));
		},
		/*ProcessSource*/[](FHarvesterFragment& SourceFragment, float Value) -> void
		{
			SourceFragment.CurrentResources += Value;
		},
		/*ProcessTarget*/[](FCollectableResourceFragment& TargetFragment, float Value) -> void
		{
			TargetFragment.CurrentAmount -= Value;
		});
}

void UHarvesterMineResourceProcessor::StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal,
	FMassExecutionContext& Context, FMassEntityHandle EntityHandle)
{
	Context.Defer().RemoveTag<FMassHarvesterStateMiningResourceTag>(EntityHandle);
	EntitiesToSignal.Add(EntityHandle);
}
