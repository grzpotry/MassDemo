#pragma once
#include "MassEntityTypes.h"
#include "MassStateTreeTypes.h"
#include "MassAddTagForDurationTask.generated.h"
/**
 * Task to assign a LookAt target for mass processing
 */
USTRUCT()
struct MASSDEMO_API FMassAddTagForDurationTaskInstanceData
{
	GENERATED_BODY()
	
	// UPROPERTY(EditAnywhere, Category = Input, meta = (Optional))
	// FMassEntityHandle TargetEntity;
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	FMassTag Tag;
};

USTRUCT(meta = (DisplayName = "Mass Testtttt Tag"))
struct MASSDEMO_API FMassAddTagForDurationTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassAddTagForDurationTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

