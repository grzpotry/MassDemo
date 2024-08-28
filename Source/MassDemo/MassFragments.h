// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Data/ResourceTemplate.h"
#include "MassFragments.generated.h"

USTRUCT()
struct FSimpleMovementFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FVector Target;
};

//eg Tree: 50 wood
//eg Rock: 20 stone
USTRUCT()
struct FCollectableResourceFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	
	TSoftObjectPtr<UResourceTemplate> Template;
	
	FVector WorldPosition;
	float CurrentAmount;
	float MaxAmount;
};


USTRUCT()
struct FHarvesterTargetFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FVector WorldPosition;
};


//tags

UENUM()
enum class EMassCustomTagAction : int8
{
	Check,
	Add,
	Remove,
};

UENUM()
enum class EMassCustomTag : int8
{
	None = 0,
	HarvesterStateIdle = 1,
	HarvesterStateSearchingTarget = 2,
	HarvesterStateMoving = 4,
	HarvesterStateInteracting = 5,
};

// Identifies harvester - collecting resources
USTRUCT()
struct FMassAgentHarvesterTag : public FMassTag
{
	GENERATED_BODY()
};


// Identifies collectable resource mined by harvesters
USTRUCT()
struct FMassCollectableResourceTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FMassHarvesterStateIdleTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FMassHarvesterStateSearchingTargetTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FMassHarvesterStateMovingTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FMassHarvesterStateInteractingTag : public FMassTag
{
	GENERATED_BODY()
};


