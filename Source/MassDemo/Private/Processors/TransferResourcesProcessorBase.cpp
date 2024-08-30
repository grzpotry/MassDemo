// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/TransferResourcesProcessorBase.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "MassDemo/MassFragments.h"

// explicitly instantiate template to avoid linker errors
template void UTransferResourcesProcessorBase::ExecuteInternal<TTransferResourceToHarvester, FHarvesterFragment, FCollectableResourceFragment>(FMassEntityManager&, FMassExecutionContext&, ETransferActionMode);

UTransferResourcesProcessorBase::UTransferResourcesProcessorBase():
	SourceQuery(*this),
	TargetQuery(*this)
{
	// not auto-registering to manually control execution
	bAutoRegisterWithProcessingPhases = false;
}

void UTransferResourcesProcessorBase::ConfigureQueries()
{
	ProcessorRequirements.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UTransferResourcesProcessorBase::StopMining(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle)
{
	Context.Defer().RemoveTag<FMassHarvesterStateMiningResourceTag>(EntityHandle);
	EntitiesToSignal.Add(EntityHandle);
}

void UTransferResourcesProcessorBase::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
}

//source - harvester
//target - resource / magazine
template <class TTransferAction, typename TSourceFragment, typename TTargetFragment>
void UTransferResourcesProcessorBase::ExecuteInternal(FMassEntityManager& EntityManager, FMassExecutionContext& Context, ETransferActionMode TransferMode)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	TMap<FMassEntityHandle, TTransferAction> TransferActions;
	TArray<FMassEntityHandle> EntitiesToSignal;
	
	SourceQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, &TransferActions, &EntitiesToSignal, TransferMode](FMassExecutionContext& _Context)
	{

		const FHarvesterConfigSharedFragment& HarvesterConfigSharedFragment = _Context.GetConstSharedFragment<FHarvesterConfigSharedFragment>();
		const float TransferSpeed = HarvesterConfigSharedFragment.MiningResourceSpeed;
		
		const TArrayView<TSourceFragment> SourcesList = _Context.GetMutableFragmentView<TSourceFragment>();
		const TArrayView<FTransferFragment> SourceTransfersList = _Context.GetMutableFragmentView<FTransferFragment>();
		
		const int32 NumEntities = _Context.GetNumEntities();
		const float CurrentTime = _Context.GetWorld()->TimeSeconds; //TODO: cache miss ?

		TransferActions.Reserve(TransferActions.Num() + NumEntities);

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			TSourceFragment& SourceFragment = SourcesList[EntityIndex];
			FTransferFragment& TransferFragment = SourceTransfersList[EntityIndex];
			const FMassEntityHandle SourceEntity = _Context.GetEntity(EntityIndex);
			EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);

			// if (!HarvesterFragment.MoveTargetEntityHandle.IsValid())
			// {
			// 	StopMining(EntitiesToSignal, _Context, HarvesterEntity);
			// 	continue;
			// }
			//
			// if (TargetFragment.CurrentResources >= TargetCapacity)
			// {
			// 	StopMining(EntitiesToSignal, _Context, TargetEntity);
			// 	continue;
			// }

			float& LastTransferTime = TransferFragment.LastTransferTime;

			// mining cooldown
			if (CurrentTime - LastTransferTime <= TransferSpeed)
			{
				continue;
			}

			TTransferResourceToHarvester TransferResource(TransferSpeed, TransferMode);
			auto TargetEntity = TransferResource.GetTransferTargetEntity(SourceFragment);
			FMassEntityView TargetEntityView(EntityManager, TargetEntity);
			TTargetFragment& TargetFragment = TargetEntityView.GetFragmentData<TTargetFragment>();

			float MaxValue = TransferSpeed;
			switch (TransferMode)
			{
				case ETransferActionMode::FromSourceToTarget:
					MaxValue = TransferResource.GetCurrentSourceValue(SourceFragment);
					break;
				case ETransferActionMode::FromTargetToSource: //target-> resource, source -> harvester
					MaxValue = TransferResource.GetCurrentTargetValue(TargetFragment);
					break;
			}

			const float TransferValue = FMath::Min(MaxValue, TransferSpeed);
			auto ClampedTransfer = TTransferResourceToHarvester(TransferValue, TransferMode);

			UE_LOG(LogTemp, Log, TEXT("Resource before clamp %f actual %f"), TransferSpeed, TransferValue);

			if (!ClampedTransfer.IsValid())
			{
				StopMining(EntitiesToSignal, _Context, SourceEntity);
				continue;
			}
			
			ClampedTransfer.ProcessSource(SourcesList, EntityIndex);
			TransferActions.Add(TargetEntity, ClampedTransfer);
			LastTransferTime = CurrentTime;
		}
	});
	
	
	TargetQuery.ForEachEntityChunk(EntityManager, Context, [this, &TransferActions](FMassExecutionContext& _Context)
	{
		const int32 NumEntities = _Context.GetNumEntities();
		const TArrayView<TTargetFragment> TargetsList = _Context.GetMutableFragmentView<TTargetFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FMassEntityHandle TargetEntity = _Context.GetEntity(EntityIndex);
			
			if (TransferActions.Contains(TargetEntity))
			{
				TTransferAction TransferAction = TransferActions[TargetEntity];
				TransferAction.ProcessTarget(TargetsList, EntityIndex);
			}
		}
	});

	TransferActions.Reset();
	
	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("harvesting completed for %i entities"), EntitiesToSignal.Num());
	}
}

template <typename TSourceFragment, typename TTargetFragment, typename TElement>
TElement UTransferResourcesProcessorBase::Clamp(float TransferValue, ETransferActionMode TransferMode,
	TSourceFragment SourceFragment, TTargetFragment TargetFragment)
{
	UE_LOG(LogTemp, Log, TEXT("clamp"));
	TElement MaxValue = TransferValue;
		
	switch (TransferMode)
	{
	case ETransferActionMode::FromSourceToTarget:
		MaxValue = GetCurrentValue(SourceFragment);
		break;
	case ETransferActionMode::FromTargetToSource: //target-> resource, source -> harvester
		MaxValue = GetCurrentValue(TargetFragment);
		UE_LOG(LogTemp, Log, TEXT("Current target val %f"), MaxValue);
		break;
	}

	return FMath::Min(MaxValue, TransferValue);
}
