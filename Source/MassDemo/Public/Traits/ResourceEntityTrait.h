// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Data/ResourceTemplate.h"
#include "ResourceEntityTrait.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UResourceEntityTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()


	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UResourceTemplate> Template;
};
