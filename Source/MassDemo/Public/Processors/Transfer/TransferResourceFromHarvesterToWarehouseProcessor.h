// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Processors/Transfer/TransferResourcesProcessorBase.h"
#include "TransferResourceFromHarvesterToWarehouseProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UTransferResourceFromHarvesterToWarehouseProcessor : public UTransferResourcesProcessorBase
{
	GENERATED_BODY()
	
	UTransferResourceFromHarvesterToWarehouseProcessor();

public:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle) override;
	
};
