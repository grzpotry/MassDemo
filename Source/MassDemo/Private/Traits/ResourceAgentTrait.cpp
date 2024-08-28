// Fill out your copyright notice in the Description page of Project Settings.


#include "Traits/ResourceAgentTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassDemo/MassFragments.h"

void UResourceAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMassCollectableResourceTag>();
	BuildContext.AddFragment<FCollectableResourceFragment>();
}
