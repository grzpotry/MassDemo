// Fill out your copyright notice in the Description page of Project Settings.


#include "Traits/HarvesterAgentTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassDemo/MassFragments.h"

void UHarvesterAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FHarvesterFragment>();
	BuildContext.AddTag<FMassAgentHarvesterTag>();
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct ConfigFragment = EntityManager.GetOrCreateConstSharedFragment(HarvesterConfig);
	BuildContext.AddConstSharedFragment(ConfigFragment);
}
