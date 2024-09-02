// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/Transfer/TransferResourcesProcessorBase.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "MassDemo/MassFragments.h"

// explicitly instantiate template to avoid linker errors
template void UTransferResourcesProcessorBase::ExecuteInternal<FHarvesterFragment, FCollectableResourceFragment, float>(FMassEntityManager&, FMassExecutionContext&,
	std::function<float(FMassExecutionContext&, FHarvesterFragment)>,
	std::function<FTransferEntityFloat(FHarvesterFragment, float)>,
	std::function<void(FHarvesterFragment&, float, FMassEntityHandle, FMassExecutionContext&)>,
	std::function<void(FCollectableResourceFragment&, float)>);

template void UTransferResourcesProcessorBase::ExecuteInternal<FHarvesterFragment, FResourcesWarehouseFragment, float>(FMassEntityManager&, FMassExecutionContext&,
	std::function<float(FMassExecutionContext&, FHarvesterFragment)>,
	std::function<FTransferEntityFloat(FHarvesterFragment, float)>,
	std::function<void(FHarvesterFragment&, float, FMassEntityHandle, FMassExecutionContext&)>,
	std::function<void(FResourcesWarehouseFragment&, float)>);

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

void UTransferResourcesProcessorBase::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
}

//TODO: verify lambdas performances, if noticeable - replace with eg. templated containers with entity mutate logic
template<Derived<FMassFragment> TSourceFragment, Derived<FMassFragment> TTargetFragment, typename TTransferrableElement>
void UTransferResourcesProcessorBase::ExecuteInternal(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	std::function<TTransferrableElement(FMassExecutionContext&, TSourceFragment)> GetTransferValue,
	std::function<FTransferEntityFloat(TSourceFragment, TTransferrableElement)> ClampTransferValue,
	std::function<void(TSourceFragment&, TTransferrableElement, FMassEntityHandle, FMassExecutionContext&)> ProcessSource,
	std::function<void(TTargetFragment&, TTransferrableElement)> ProcessTarget)
{
	UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
	TMap<FMassEntityHandle, FTransferEntityFloat> TransferActions;
	TArray<FMassEntityHandle> EntitiesToSignal;
	
	SourceQuery.ForEachEntityChunk(EntityManager, Context, [this, &TransferActions, &EntitiesToSignal, &ProcessSource, &ClampTransferValue, &GetTransferValue](FMassExecutionContext& _Context)
	{
		const TArrayView<TSourceFragment> SourcesList = _Context.GetMutableFragmentView<TSourceFragment>();
		const TArrayView<FTransferFragment> SourceTransfersList = _Context.GetMutableFragmentView<FTransferFragment>();
		
		const int32 NumEntities = _Context.GetNumEntities();
		const float CurrentTime = _Context.GetWorld()->TimeSeconds; //TODO: cache miss ?

		TransferActions.Reserve(TransferActions.Num() + NumEntities);

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			TSourceFragment& SourceFragment = SourcesList[EntityIndex];
			const float TransferSpeed = GetTransferValue(_Context, SourceFragment);
			FTransferFragment& TransferFragment = SourceTransfersList[EntityIndex];
			const FMassEntityHandle SourceEntity = _Context.GetEntity(EntityIndex);
			EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);
			
			float& LastTransferTime = TransferFragment.LastTransferTime;

			// transfer cooldown
			if (CurrentTime - LastTransferTime <= TransferSpeed)
			{
				continue;
			}
			
			FTransferEntityFloat TransferValue = ClampTransferValue(SourceFragment, TransferSpeed);

			if (!TransferValue.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("invalid transfer, stop mining"));
				StopTransfer(EntitiesToSignal, _Context, SourceEntity);
				continue;
			}

			ProcessSource(SourcesList[EntityIndex], TransferValue.Value, SourceEntity, _Context);
			TransferActions.Add(TransferValue.TargetEntity, TransferValue);
			LastTransferTime = CurrentTime;
		}
	});

	if (!TransferActions.IsEmpty())
	{
		TargetQuery.ForEachEntityChunk(EntityManager, Context, [this, &TransferActions, &ProcessTarget](FMassExecutionContext& _Context)
		{
			const int32 NumEntities = _Context.GetNumEntities();
			const TArrayView<TTargetFragment> TargetsList = _Context.GetMutableFragmentView<TTargetFragment>();
				
			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				FMassEntityHandle TargetEntity = _Context.GetEntity(EntityIndex);
					
				if (TransferActions.Contains(TargetEntity))
				{
					const auto TransferEntityFloat = TransferActions[TargetEntity];
					float TransferValue = TransferEntityFloat.Value;
					ProcessTarget(TargetsList[EntityIndex], TransferValue);
				}
			}
		});
	}

	TransferActions.Reset();
	
	// Signal all entities inside the consolidated list
	if (EntitiesToSignal.Num())
	{
		//Tick state trees
		SignalSubsystem.SignalEntitiesDeferred(Context, UE::Mass::Signals::NewStateTreeTaskRequired, EntitiesToSignal);
		UE_LOG(LogTemp, Log, TEXT("harvesting completed for %i entities"), EntitiesToSignal.Num());
	}
}
