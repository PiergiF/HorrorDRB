// Copyright Metaxis Games 2026, All Rights Reserved

#include "MetaxisTeamSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

void UMetaxisTeamSubsystem::PostInitialize()
{
	Super::PostInitialize();

	FGenericTeamId::SetAttitudeSolver([WeakThis = MakeWeakObjectPtr(this)](FGenericTeamId A, FGenericTeamId B)
	{
		if (WeakThis.IsValid())
		{
			return WeakThis->ResolveAttitude(A, B);
		}
		return A != B ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
	});
}

void UMetaxisTeamSubsystem::Deinitialize()
{
	FGenericTeamId::ResetAttitudeSolver();
	Super::Deinitialize();
}

bool UMetaxisTeamSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMetaxisTeamSubsystem::SetTeamRelationship(uint8 TeamA, uint8 TeamB, TEnumAsByte<ETeamAttitude::Type> NewAttitude)
{
	for (FMetaxisTeamRelationship& Existing : TeamRelationships)
	{
		if ((Existing.TeamA == TeamA && Existing.TeamB == TeamB)
			|| (Existing.TeamA == TeamB && Existing.TeamB == TeamA))
		{
			Existing.Attitude = NewAttitude;
			UE_LOG(LogTemp, Log, TEXT("SetTeamRelationship: Updated relationship between Team %d and Team %d to %s."),
				TeamA, TeamB, *UEnum::GetValueAsString(NewAttitude.GetValue()));
			return;
		}
	}

	FMetaxisTeamRelationship NewEntry;
	NewEntry.TeamA = TeamA;
	NewEntry.TeamB = TeamB;
	NewEntry.Attitude = NewAttitude;
	TeamRelationships.Add(NewEntry);

	UE_LOG(LogTemp, Log, TEXT("SetTeamRelationship: Added relationship between Team %d and Team %d as %s."),
		TeamA, TeamB, *UEnum::GetValueAsString(NewAttitude.GetValue()));
}

TEnumAsByte<ETeamAttitude::Type> UMetaxisTeamSubsystem::GetTeamRelationship(uint8 TeamA, uint8 TeamB) const
{
	return ResolveAttitude(FGenericTeamId(TeamA), FGenericTeamId(TeamB));
}

bool UMetaxisTeamSubsystem::RemoveTeamRelationship(uint8 TeamA, uint8 TeamB)
{
	for (int32 i = TeamRelationships.Num() - 1; i >= 0; --i)
	{
		const FMetaxisTeamRelationship& Entry = TeamRelationships[i];
		if ((Entry.TeamA == TeamA && Entry.TeamB == TeamB)
			|| (Entry.TeamA == TeamB && Entry.TeamB == TeamA))
		{
			TeamRelationships.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("RemoveTeamRelationship: Removed relationship between Team %d and Team %d."), TeamA, TeamB);
			return true;
		}
	}
	return false;
}

void UMetaxisTeamSubsystem::LoadRelationshipsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadRelationshipsFromDataTable: Invalid DataTable provided."));
		return;
	}

	TArray<FMetaxisTeamRelationship*> Rows;
	DataTable->GetAllRows<FMetaxisTeamRelationship>(TEXT("UMetaxisTeamSubsystem::LoadRelationshipsFromDataTable"), Rows);

	for (const FMetaxisTeamRelationship* Row : Rows)
	{
		if (Row)
		{
			SetTeamRelationship(Row->TeamA.GetId(), Row->TeamB.GetId(), Row->Attitude);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LoadRelationshipsFromDataTable: Loaded %d rows from DataTable %s."), Rows.Num(), *DataTable->GetName());
}

void UMetaxisTeamSubsystem::ClearAllRelationships()
{
	TeamRelationships.Empty();
	UE_LOG(LogTemp, Log, TEXT("ClearAllRelationships: All team relationships cleared."));
}

ETeamAttitude::Type UMetaxisTeamSubsystem::ResolveAttitude(FGenericTeamId A, FGenericTeamId B) const
{
	for (const FMetaxisTeamRelationship& Relationship : TeamRelationships)
	{
		if ((Relationship.TeamA == A && Relationship.TeamB == B)
			|| (Relationship.TeamA == B && Relationship.TeamB == A))
		{
			return Relationship.Attitude;
		}
	}

	return A != B ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}
