// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TransferResourcesProcessorBase.h"
#include "TransferResourceToHarvesterProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UTransferResourceToHarvesterProcessor : public UTransferResourcesProcessorBase
{
	GENERATED_BODY()

	UTransferResourceToHarvesterProcessor();

public:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void StopTransfer(TArray<FMassEntityHandle>& EntitiesToSignal, FMassExecutionContext& Context, FMassEntityHandle EntityHandle) override;
};
