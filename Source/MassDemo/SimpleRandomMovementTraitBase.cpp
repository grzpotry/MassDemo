// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleRandomMovementTraitBase.h"

#include "MassEntityTemplateRegistry.h"
#include "MassFragments.h"

void USimpleRandomMovementTraitBase::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
                                                   const UWorld& World) const
{
	BuildContext.AddFragment<FSimpleMovementFragment>();
}
