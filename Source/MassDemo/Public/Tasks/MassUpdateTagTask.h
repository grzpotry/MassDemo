#pragma once
#include "MassEntityTypes.h"
#include "MassExecutionContext.h"
#include "MassStateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"
#include "MassDemo/MassFragments.h"
#include "MassUpdateTagTask.generated.h"

/**
 * Task to assign a LookAt target for mass processing
 */
USTRUCT()
struct MASSDEMO_API FMassAddTagForDurationTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	EMassCustomTag Tag = EMassCustomTag::None;

	/** Delay before the task ends. Default (0 or any negative) will run as long as tag is present - requires a transition in the state tree to stop it.
	 * With positive duration, tag will be removed automatically when time passed.
	 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float Duration = 0.f;

	/** Accumulated time used to stop task if duration is set */
	UPROPERTY()
	float Time = 0.f;
};


/*
 * Ensure specific tag exists on entity while state is running
 */
USTRUCT(meta = (DisplayName = "Mass Update Tag"))
struct MASSDEMO_API FMassUpdateTagTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassAddTagForDurationTaskInstanceData;

public:
	static bool ProcessEntityTag(const EMassCustomTag Tag, FMassExecutionContext& MassExecutionContext,
	                             const FMassEntityHandle EntityHandle, EMassCustomTagAction TagAction);

	FMassUpdateTagTask();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	bool ProcessEntityTag(FStateTreeExecutionContext& Context, EMassCustomTagAction TagAction) const;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	template <typename T>
	static bool ProcessEntityTag(FMassEntityHandle EntityHandle, FMassExecutionContext& Context,
	                             EMassCustomTagAction TagAction);

		TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};

