// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

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

USTRUCT()
struct FTransferEntityFloat
{
	GENERATED_BODY()

	FMassEntityHandle TargetEntity;
	float Value;

	FTransferEntityFloat() = default;

	bool IsValid()
	{
		return Value > 0 && TargetEntity.IsValid();
	}

	static FTransferEntityFloat Invalid()
	{
		return FTransferEntityFloat(FMassEntityHandle(), 0);
	}

	FTransferEntityFloat(FMassEntityHandle Entity, float Element) :
		TargetEntity(Entity),
		Value(Element)
	{
	};
};

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

UCLASS(abstract)
class MASSDEMO_API UTransferResourcesProcessorBase : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTransferResourcesProcessorBase();

	virtual void ConfigureQueries() override;
	virtual void StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle)PURE_VIRTUAL(UTransferResourcesProcessorBase::StopTransfer);

	template<Derived<FMassFragment> TSourceFragment, Derived<FMassFragment> TTargetFragment, typename TTransferrableElement>
	void ExecuteInternal(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	                     std::function<TTransferrableElement(FMassExecutionContext&, TSourceFragment)> GetTransferValue,
	                     std::function<FTransferEntityFloat(TSourceFragment, TTransferrableElement)> ClampTransferValue,
	                     std::function<void(TSourceFragment&, TTransferrableElement, FMassEntityHandle, FMassExecutionContext& Context)> ProcessSource,
	                     std::function<void(TTargetFragment&, TTransferrableElement)> ProcessTarget);

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	
protected:
	FMassEntityQuery SourceQuery;
	FMassEntityQuery TargetQuery;
};
