// Copyright Metaxis Games 2026, All Rights Reserved

#include "MetaxisTeamComponent.h"
#include "GenericTeamAgentInterface.h"

UMetaxisTeamComponent::UMetaxisTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TeamID = FGenericTeamId::NoTeam;
}

void UMetaxisTeamComponent::SwitchTeam(uint8 NewTeamID)
{
	TeamID = NewTeamID;
	UE_LOG(LogTemp, Log, TEXT("UMetaxisTeamComponent '%s': Switched to Team %d on actor %s."),
		*GetName(), NewTeamID, *GetOwner()->GetName());
}
