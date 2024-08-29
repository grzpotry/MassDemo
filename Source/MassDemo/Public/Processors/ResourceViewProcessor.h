// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ResourceViewProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UResourceViewProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UResourceViewProcessor();

private:
	virtual void ConfigureQueries() override;
	virtual auto Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) -> void override;

private:
	FMassEntityQuery EntityQuery;
};
