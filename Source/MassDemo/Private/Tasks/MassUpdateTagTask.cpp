
#include "..\..\Public\Tasks\MassUpdateTagTask.h"
#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassDemo/MassFragments.h"

bool FMassUpdateTagTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

EStateTreeRunStatus FMassUpdateTagTask::EnterState(FStateTreeExecutionContext& Context,
                                                   const FStateTreeTransitionResult& Transition) const
{
	UE_LOG(LogTemp, Log, TEXT("Entered state %s"), *Context.GetActiveStateName());
	ProcessEntityTag(Context, EMassCustomTagAction::Add);
	return EStateTreeRunStatus::Running;
}


void FMassUpdateTagTask::ExitState(FStateTreeExecutionContext& Context,
                                   const FStateTreeTransitionResult& Transition) const
{
	UE_LOG(LogTemp, Log, TEXT("Exited state %s"), *Context.GetActiveStateName());
	//ProcessEntityTag(Context, EMassCustomTagAction::Remove);
}

EStateTreeRunStatus FMassUpdateTagTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const bool bIsTagPresent = ProcessEntityTag(Context, EMassCustomTagAction::Check);
	return bIsTagPresent ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

bool FMassUpdateTagTask::ProcessEntityTag(FStateTreeExecutionContext& Context, EMassCustomTagAction TagAction) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassExecutionContext& MassExecutionContext = MassContext.GetEntitySubsystemExecutionContext();
	const FMassEntityHandle EntityHandle = MassContext.GetEntity();

	if (!EntityHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid entity"));
		return false;
	}

	return ProcessEntityTag(InstanceData.Tag, MassExecutionContext, EntityHandle, TagAction);
}

bool FMassUpdateTagTask::ProcessEntityTag(const EMassCustomTag Tag, FMassExecutionContext& MassExecutionContext,
                                          const FMassEntityHandle EntityHandle, EMassCustomTagAction
                                          TagAction)
{
	switch (Tag)
	{
		case EMassCustomTag::None:
			return false;
		case EMassCustomTag::HarvesterStateIdle:
			return ProcessEntityTag<FMassHarvesterStateIdleTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateMoving:
			return ProcessEntityTag<FMassHarvesterStateMovingTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateInteracting:
			return ProcessEntityTag<FMassHarvesterStateInteractingTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateSearchingTarget:
			return ProcessEntityTag<FMassHarvesterStateSearchingTargetTag>(EntityHandle, MassExecutionContext, TagAction);
	default:
			checkf(false, TEXT("Custom mass tag not supported"));
			return false;
	}
}

template <typename T>
bool FMassUpdateTagTask::ProcessEntityTag(FMassEntityHandle EntityHandle, FMassExecutionContext& Context,
                                         EMassCustomTagAction TagAction)
{
	switch (TagAction)
	{
		case EMassCustomTagAction::Check:
			return Context.DoesArchetypeHaveTag<T>();
		case EMassCustomTagAction::Add:
			//if (!Context.DoesArchetypeHaveTag<T>())
			{
				Context.Defer().AddTag<T>(EntityHandle);
				UE_LOG(LogTemp, Log, TEXT("Added tag"));
			}
			return true;
		case EMassCustomTagAction::Remove:
			//if (Context.DoesArchetypeHaveTag<T>())
			{
				Context.Defer().RemoveTag<T>(EntityHandle);
			}
			return false;
	default:
		return false;
	}
}
