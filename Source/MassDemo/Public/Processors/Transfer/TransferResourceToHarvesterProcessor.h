// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TransferResourcesProcessorBase.h"
#include "HarvesterMineResourceProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UHarvesterMineResourceProcessor : public UTransferResourcesProcessorBase
{
	GENERATED_BODY()

	UHarvesterMineResourceProcessor();

public:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle) override;
};
