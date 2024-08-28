// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HarvesterMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UHarvesterMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UHarvesterMovementProcessor();
	
	virtual void ConfigureQueries() override;
	virtual auto Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) -> void override;
	
private:
	FMassEntityQuery EntityQuery;
};
