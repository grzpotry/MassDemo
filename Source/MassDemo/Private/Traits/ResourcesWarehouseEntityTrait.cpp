// Fill out your copyright notice in the Description page of Project Settings.


#include "Traits/ResourcesWarehouseEntityTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassDemo/MassFragments.h"

void UResourcesWarehouseEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
                                                   const UWorld& World) const
{
	BuildContext.AddTag<FMassEntityResourcesWarehouseTag>();
	BuildContext.AddFragment<FResourcesWarehouseFragment>();
}
