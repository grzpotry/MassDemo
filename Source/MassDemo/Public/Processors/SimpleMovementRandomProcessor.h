// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "SimpleMovementRandomProcessor.generated.h"


/**
 * 
 */
UCLASS()
class MASSDEMO_API USimpleMovementRandomProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	USimpleMovementRandomProcessor();
	
protected:
	virtual void ConfigureQueries() override;
	virtual auto Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) -> void override;

private:
	FMassEntityQuery EntityQuery;
	
};
