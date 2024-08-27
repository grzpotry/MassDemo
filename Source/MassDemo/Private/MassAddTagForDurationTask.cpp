#include "MassAddTagForDurationTask.h"
#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassDemo/MassFragments.h"

bool FMassAddTagForDurationTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

EStateTreeRunStatus FMassAddTagForDurationTask::EnterState(FStateTreeExecutionContext& Context,
                                                           const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);

	const FMassExecutionContext MassExecutionContext = MassContext.GetEntitySubsystemExecutionContext();
	const FMassEntityHandle EntityHandle = MassContext.GetEntity();

	if (!EntityHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid entity"));
		return EStateTreeRunStatus::Failed;
	}
	
	if (!MassExecutionContext.DoesArchetypeHaveTag<FMassTestTaskTag>())
	{
		MassExecutionContext.Defer().AddTag<FMassTestTaskTag>(EntityHandle);
	}
	
	return EStateTreeRunStatus::Running;
}

void FMassAddTagForDurationTask::ExitState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) const
{
}
