// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassMovementFragments.h"
#include "MassDemo/MassFragments.h"
#include "HarvesterEntityTrait.generated.h"
/**
 * 
 */
UCLASS()
class MASSDEMO_API UHarvesterEntityTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(Category="Config", EditAnywhere)
	FHarvesterConfigSharedFragment HarvesterConfig;

	UPROPERTY(Category="Movement", EditAnywhere)
	FMassMovementParameters Movement;
};
