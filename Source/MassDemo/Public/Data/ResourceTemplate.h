// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ResourceTemplate.generated.h"

/**
 * 
 */
UCLASS()
class MASSDEMO_API UResourceTemplate : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
};
