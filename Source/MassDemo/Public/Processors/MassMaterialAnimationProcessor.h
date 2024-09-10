// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassMaterialAnimationProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UMassMaterialAnimationProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMassMaterialAnimationProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;	
};
