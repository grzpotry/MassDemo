// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityView.h"
#include "MassProcessor.h"
#include "MassDemo/MassFragments.h"
#include "TransferResourcesProcessorBase.generated.h"

USTRUCT()
struct FMiningAction
{
	GENERATED_BODY()

	FMiningAction() = default;

	FMiningAction(float Amount)
		: Amount(Amount)
	{
	}

	float Amount;
};

UENUM()
enum class ETransferActionMode : int8
{
	FromSourceToTarget = 0,
	FromTargetToSource = 1,
};

template<typename TSourceFragment, typename TTargetFragment, typename TElement>
class TTransferAction
{

public :

	// TSourceFragment SourceFragment;
	// TTargetFragment TargetFragment;
	TElement TransferValue;
	ETransferActionMode TransferMode;

	TTransferAction() = default;

	bool IsValid()
	{
		return TransferValue > 0;
	}

	TTransferAction(TElement Value, ETransferActionMode Mode)//, TSourceFragment SourceFragment, TTargetFragment TargetFragment):
		: TransferValue(Value),
		 TransferMode(Mode)
	{
	}

	virtual FMassEntityHandle GetTransferTargetEntity(TSourceFragment SourceFragment) =0;

	TElement Clamp(TSourceFragment SourceFragment, TTargetFragment TargetFragment)
	{
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

	virtual TElement& GetCurrentTargetValue(TTargetFragment& Fragment) = 0;
	virtual TElement& GetCurrentSourceValue(TSourceFragment& Fragment) = 0;

	virtual void ProcessTarget(const TArrayView<TTargetFragment>& TargetsView, int32 EntityIndex) = 0;
	virtual void ProcessSource(const TArrayView<TSourceFragment>& SourcesView, int32 EntityIndex) = 0;

	virtual ~TTransferAction() = default;

private:
	TElement GetTargetCapacity(const FMassEntityView& TargetEntityView)
	{
		TTargetFragment ResourceFragment = TargetEntityView.GetFragmentData<TTargetFragment>();
		return GetTargetValue(ResourceFragment);
	}
};

class TTransferResourceToHarvester : public TTransferAction<FHarvesterFragment, FCollectableResourceFragment, float>
{
	

public:
	virtual ~TTransferResourceToHarvester() override = default;

	virtual void ProcessTarget(const TArrayView<FCollectableResourceFragment>& TargetsView, int32 EntityIndex) override
	{
		const float Mul = TransferMode == ETransferActionMode::FromSourceToTarget ? 1 : -1;
		GetCurrentTargetValue(TargetsView[EntityIndex]) += TransferValue * Mul;
		UE_LOG(LogTemp, Log, TEXT("Process target %f"), TransferValue * Mul);
	}

	virtual void ProcessSource(const TArrayView<FHarvesterFragment>& SourcesView, int32 EntityIndex) override
	{
		const float Mul = TransferMode == ETransferActionMode::FromTargetToSource ? 1 : -1;
		GetCurrentSourceValue(SourcesView[EntityIndex]) += TransferValue * Mul;
		UE_LOG(LogTemp, Log, TEXT("Process source %f"), TransferValue * Mul);
	}

	TTransferResourceToHarvester(float MinedResources, ETransferActionMode Mode)
		: TTransferAction(MinedResources, Mode)
	{
	}

	virtual float& GetCurrentSourceValue(FHarvesterFragment& Fragment) override
	{
		return Fragment.CurrentResources;
	}

	virtual float& GetCurrentTargetValue(FCollectableResourceFragment& Fragment) override
	{
		return Fragment.CurrentAmount;
	}

	virtual FMassEntityHandle GetTransferTargetEntity(FHarvesterFragment SourceFragment) override
	{
		return SourceFragment.MoveTargetEntityHandle;
	}
};

/**
 * 
 */

UCLASS(abstract)
class MASSDEMO_API UTransferResourcesProcessorBase : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTransferResourcesProcessorBase();

	virtual void ConfigureQueries() override;
	static void StopMining(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle);

	
	template <class TTransferAction, typename TSourceFragment, typename TTargetFragment>
	void ExecuteInternal(FMassEntityManager& EntityManager, FMassExecutionContext& Context, ETransferActionMode
	                     TransferMode);

	template <typename TSourceFragment, typename TTargetFragment, typename TElement>
	TElement Clamp(float TransferValue, ETransferActionMode TransferMode, TSourceFragment SourceFragment, TTargetFragment TargetFragment);

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	
protected:
	FMassEntityQuery SourceQuery;
	FMassEntityQuery TargetQuery;
};
