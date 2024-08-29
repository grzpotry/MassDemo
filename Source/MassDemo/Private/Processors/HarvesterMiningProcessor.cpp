// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/HarvesterMiningProcessor.h"

#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "MassDemo/MassFragments.h"

void UHarvesterMiningProcessor::ConfigureQueries()
{
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	
	HarvesterQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	HarvesterQuery.AddRequirement<FHarvesterFragment>(EMassFragmentAccess::ReadWrite);
	HarvesterQuery.AddConstSharedRequirement<FHarvesterConfigSharedFragment>(EMassFragmentPresence::All);
	HarvesterQuery.AddTagRequirement<FMassAgentHarvesterTag>(EMassFragmentPresence::All);
	HarvesterQuery.AddTagRequirement<FMassHarvesterStateMiningResourceTag>(EMassFragmentPresence::All);
	HarvesterQuery.RegisterWithProcessor(*this);

	ResourceQuery.AddRequirement<FCollectableResourceFragment>(EMassFragmentAccess::ReadWrite);
	ResourceQuery.AddTagRequirement<FMassCollectableResourceTag>(EMassFragmentPresence::All);
	ResourceQuery.RegisterWithProcessor(*this);
}

void UHarvesterMiningProcessor::StopMining(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle)
{
	Context.Defer().RemoveTag<FMassHarvesterStateMiningResourceTag>(EntityHandle); // stop mining
	EntitiesToSignal.Add(EntityHandle);
}

void UHarvesterMiningProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	
	//todo: store as member
	TMap<FMassEntityHandle, FMiningAction> MiningActions;
	TArray<FMassEntityHandle> EntitiesToSignal;
	
	HarvesterQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, &MiningActions, &EntitiesToSignal](FMassExecutionContext& _Context)
	{
		const FHarvesterConfigSharedFragment& HarvesterConfigSharedFragment = _Context.GetConstSharedFragment<FHarvesterConfigSharedFragment>();
		const TArrayView<FHarvesterFragment> HarvestersList = _Context.GetMutableFragmentView<FHarvesterFragment>();
		const float MiningSpeed = HarvesterConfigSharedFragment.MiningSpeed;
		const int32 NumEntities = _Context.GetNumEntities();
		const float CurrentTime = _Context.GetWorld()->TimeSeconds; //TODO: cache miss ?

		MiningActions.Reserve(MiningActions.Num() + NumEntities);

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FHarvesterFragment& HarvesterFragment = HarvestersList[EntityIndex];
			FMassEntityHandle HarvesterEntity = _Context.GetEntity(EntityIndex);
			EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);

			if (!HarvesterFragment.MoveTargetEntityHandle.IsValid())
			{
				StopMining(EntitiesToSignal, _Context, HarvesterEntity);
				continue;
			}

			if (HarvesterFragment.CurrentResources >= HarvesterConfigSharedFragment.ResourcesStorageCapacity)
			{
				StopMining(EntitiesToSignal, _Context, HarvesterEntity);
				continue;
			}

			FMassEntityView ResourceEntityView(EntityManager, HarvesterFragment.MoveTargetEntityHandle);
			FCollectableResourceFragment ResourceFragment = ResourceEntityView.GetFragmentData<FCollectableResourceFragment>();
			float& LastMiningTime = HarvesterFragment.LastMiningTime;
			
			if (ResourceFragment.CurrentAmount == 0.0)
			{
				StopMining(EntitiesToSignal, _Context, HarvesterEntity);
				continue;
			}

			// mining cooldown
			if (CurrentTime - HarvesterFragment.LastMiningTime <= MiningSpeed)
			{
				continue;
			}

			LastMiningTime = CurrentTime;

			const float MinedResources = FMath::Min(ResourceFragment.CurrentAmount, HarvesterConfigSharedFragment.MiningSpeed);
			
			MiningActions.Add(HarvesterFragment.MoveTargetEntityHandle, FMiningAction(MinedResources));
			HarvesterFragment.CurrentResources += MinedResources;

			UE_LOG(LogTemp, Log, TEXT("added resource6"));
		}
	});
	
	
	ResourceQuery.ForEachEntityChunk(EntityManager, Context, [this, &MiningActions](FMassExecutionContext& _Context)
	{
		const int32 NumEntities = _Context.GetNumEntities();
		const TArrayView<FCollectableResourceFragment> ResourcesList = _Context.GetMutableFragmentView<FCollectableResourceFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			float& TotalResources = ResourcesList[EntityIndex].CurrentAmount;
			FMassEntityHandle ResourceEntity = _Context.GetEntity(EntityIndex);
			
			if (MiningActions.Contains(ResourceEntity))
			{
				const float MiningAmount = MiningActions[ResourceEntity].Amount;
				TotalResources -= FMath::Min(TotalResources, MiningAmount);
				UE_LOG(LogTemp, Log, TEXT("mined resource. Total resources: %f"), TotalResources)
			}
		}
	});

	MiningActions.Reset();
	
	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("harvesting completed for %i entities"), EntitiesToSignal.Num());
	}
}
