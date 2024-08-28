// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Data/ResourceTemplate.h"
#include "ResourceAgentTrait.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UResourceAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()


	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UResourceTemplate> Template;
};
