#include "Conditions/MassTagConditions.h"

#include "Tasks/MassUpdateTagTask.h"

bool FMassTagConditions::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassExecutionContext& MassExecutionContext = MassContext.GetEntitySubsystemExecutionContext();
	const FMassEntityHandle EntityHandle = MassContext.GetEntity();
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	return FMassUpdateTagTask::ProcessEntityTag(InstanceData.Tag, MassExecutionContext, EntityHandle, EMassCustomTagAction::Check) ^ bInvert;
}
