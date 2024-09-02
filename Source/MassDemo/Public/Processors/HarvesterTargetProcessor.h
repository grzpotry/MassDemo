// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationSubsystem.h"
#include "MassProcessor.h"
#include "HarvesterTargetProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UHarvesterTargetProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHarvesterTargetProcessor();
	
	virtual void ConfigureQueries() override;
	virtual auto Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) -> void override;
	virtual void Initialize(UObject& Owner) override;

private:
	FMassEntityQuery EntityQuery;

	/** Size of the query to find targets */
	UPROPERTY(EditDefaultsOnly, Category = Query, config, meta = (UIMin = 0.0, ClampMin = 0.0))
	float QueryExtent = 0.f;

	void OnTargetSearchFailed(const FVector& QueryOrigin, const FVector& Extent) const;
};
