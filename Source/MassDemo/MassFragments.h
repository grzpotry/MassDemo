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


// configuration for all harvesters
USTRUCT()
struct FHarvesterConfigSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "General", meta = (ClampMin = "0.0"))
	float MoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "General", meta = (ClampMin = "0.0"))
	float TargetStopDistance = 10.0f;
};


//tags

// Describes method executed on custom Mass tag
UENUM()
enum class EMassCustomTagAction : int8
{
	Check = 0,
	Add = 1,
	Remove = 2,
};

// Custom tags used by StateTree tasks - quick solution for referencing MassTags in stateTree editor
UENUM()
enum class EMassCustomTag : int8
{
	None = 0,
	HarvesterStateSearchingTarget = 2,
	HarvesterStateMoving = 4,
	HarvesterStateInteracting = 5,
};

// Identifies harvester - agent collecting resources
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

// harvester StateTree state: SearchingTarget
USTRUCT()
struct FMassHarvesterStateSearchingTargetTag : public FMassTag
{
	GENERATED_BODY()
};

// harvester StateTree state: Moving
USTRUCT()
struct FMassHarvesterStateMovingTag : public FMassTag
{
	GENERATED_BODY()
};

// harvester StateTree state: Interacting
USTRUCT()
struct FMassHarvesterStateInteractingTag : public FMassTag
{
	GENERATED_BODY()
};


