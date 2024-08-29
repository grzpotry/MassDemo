// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HarvesterMiningProcessor.generated.h"

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

/**
 * 
 */
UCLASS()
class MASSDEMO_API UHarvesterMiningProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	virtual void ConfigureQueries() override;
	static void StopMining(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle);
	virtual auto Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) -> void override;
	
private:
	FMassEntityQuery HarvesterQuery;
	FMassEntityQuery ResourceQuery;
};
