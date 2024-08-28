#pragma once
#include "StateTreeConditionBase.h"
#include "MassDemo/MassFragments.h"
#include "MassTagConditions.generated.h"

USTRUCT()
struct MASSDEMO_API FMassTagConditionsInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	EMassCustomTag Tag = EMassCustomTag::None;
};

USTRUCT(DisplayName="Mass Tag Contains")
struct MASSDEMO_API FMassTagConditions : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassTagConditionsInstanceData;

	FMassTagConditions() = default;

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};