// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/HarvesterMineResourceProcessor.h"

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
	

	ExecuteInternal<TTransferResourceToHarvester, FHarvesterFragment, FCollectableResourceFragment>(EntityManager, Context, ETransferActionMode::FromTargetToSource);
}
