// Fill out your copyright notice in the Description page of Project Settings.


#include "Traits/HarvesterEntityTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassDemo/MassFragments.h"

void UHarvesterEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FHarvesterFragment>();
	BuildContext.AddFragment<FTransferFragment>();
	BuildContext.AddTag<FMassEntityHarvesterTag>();
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct ConfigFragment = EntityManager.GetOrCreateConstSharedFragment(HarvesterConfig);
	BuildContext.AddConstSharedFragment(ConfigFragment);

	const FConstSharedStruct MovementFragment = EntityManager.GetOrCreateConstSharedFragment(Movement);
	BuildContext.AddConstSharedFragment(MovementFragment);
}
