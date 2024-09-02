﻿
#include "..\..\Public\Tasks\MassUpdateTagTask.h"
#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassDemo/MassFragments.h"

FMassUpdateTagTask::FMassUpdateTagTask()
{
	bShouldCallTick = true;
	bShouldAffectTransitions = true;
}

bool FMassUpdateTagTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassUpdateTagTask::EnterState(FStateTreeExecutionContext& Context,
                                                   const FStateTreeTransitionResult& Transition) const
{
	UE_LOG(LogTemp, Log, TEXT("Entered state %s"), *Context.GetActiveStateName());
	ProcessEntityTag(Context, EMassCustomTagAction::Add);
	
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Time = 0;
	
	// A Duration <= 0 indicates that the task runs until a transition in the state tree stops it.
	// Otherwise we schedule a signal to end the task.
	if (InstanceData.Duration > 0.0f)
	{
		MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), InstanceData.Duration);
	}
	
	return EStateTreeRunStatus::Running;
}


void FMassUpdateTagTask::ExitState(FStateTreeExecutionContext& Context,
                                   const FStateTreeTransitionResult& Transition) const
{
	ProcessEntityTag(Context, EMassCustomTagAction::Remove);
}

EStateTreeRunStatus FMassUpdateTagTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const bool bIsTagPresent = ProcessEntityTag(Context, EMassCustomTagAction::Check);

	int tagIndex = static_cast<int>(Context.GetInstanceData(*this).Tag);
	//UE_LOG(LogTemp, Log, TEXT("Ticking state IsRunning: %i tagIndex: %i"), bIsTagPresent, tagIndex);
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.Time += DeltaTime;

	if (InstanceData.Duration > 0.0f)
	{
		return InstanceData.Time < InstanceData.Duration ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
	}
	
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
		case EMassCustomTag::HarvesterStateMoving:
			return ProcessEntityTag<FMassHarvesterStateMovingTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateInteracting:
			return ProcessEntityTag<FMassHarvesterStateInteractingTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateSearchingTarget:
			return ProcessEntityTag<FMassHarvesterStateSearchingTargetTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateMiningResource:
			return ProcessEntityTag<FMassHarvesterStateMiningResourceTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterStateDeliverResource:
        	return ProcessEntityTag<FMassHarvesterStateDeliverResourceTag>(EntityHandle, MassExecutionContext, TagAction);
		case EMassCustomTag::HarvesterIsFull:
			return ProcessEntityTag<FMassHarvesterIsFullTag>(EntityHandle, MassExecutionContext, TagAction);
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
			Context.Defer().AddTag<T>(EntityHandle);
				return true;
			case EMassCustomTagAction::Remove:
				Context.Defer().RemoveTag<T>(EntityHandle);
				return false;
		default:
			return false;
	}
}
