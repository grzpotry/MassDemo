// Fill out your copyright notice in the Description page of Project Settings.


#include "Traits/ResourceEntityTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassDemo/MassFragments.h"

void UResourceEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMassEntityCollectableResourceTag>();
	BuildContext.AddFragment<FCollectableResourceFragment>();
}
