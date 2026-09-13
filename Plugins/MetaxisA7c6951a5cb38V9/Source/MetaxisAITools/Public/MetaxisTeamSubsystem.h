// Copyright Metaxis Games 2026, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GenericTeamAgentInterface.h"
#include "MetaxisTeamRelationship.h"
#include "MetaxisTeamSubsystem.generated.h"

class UDataTable;

/**
 * World subsystem that manages team relationships and installs a custom
 * attitude solver into FGenericTeamId. This enables complex multi-team
 * relationships (e.g. Team 1 ↔ Team 2: Hostile, Team 1 ↔ Team 3: Friendly)
 * that the engine's default solver does not support.
 */
UCLASS()
class METAXISAITOOLS_API UMetaxisTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// USubsystem interface
	virtual void PostInitialize() override;
	virtual void Deinitialize() override;

	/**
	 * Set or update the relationship between two teams.
	 * The relationship is symmetric: setting TeamA→TeamB also applies to TeamB→TeamA.
	 */
	UFUNCTION(BlueprintCallable, Category = "Metaxis AI Perception Tools|Team Relationship", meta = (DisplayName = "Set Team Relationship"))
	void SetTeamRelationship(uint8 TeamA, uint8 TeamB, TEnumAsByte<ETeamAttitude::Type> NewAttitude);

	/**
	 * Get the relationship between two teams.
	 * Returns Friendly if same team, Hostile if different with no explicit relationship set.
	 */
	UFUNCTION(BlueprintPure, Category = "Metaxis AI Perception Tools|Team Relationship", meta = (DisplayName = "Get Team Relationship"))
	TEnumAsByte<ETeamAttitude::Type> GetTeamRelationship(uint8 TeamA, uint8 TeamB) const;

	/** Remove an explicitly-defined relationship between two teams, reverting to default behaviour. */
	UFUNCTION(BlueprintCallable, Category = "Metaxis AI Perception Tools|Team Relationship", meta = (DisplayName = "Remove Team Relationship"))
	bool RemoveTeamRelationship(uint8 TeamA, uint8 TeamB);

	/**
	 * Bulk-load relationships from a DataTable whose row struct is FMetaxisTeamRelationship.
	 * Existing relationships are preserved; rows in the table overwrite conflicting entries.
	 */
	UFUNCTION(BlueprintCallable, Category = "Metaxis AI Perception Tools|Team Relationship", meta = (DisplayName = "Load Relationships From DataTable"))
	void LoadRelationshipsFromDataTable(UDataTable* DataTable);

	/** Remove all explicitly-defined relationships, reverting entirely to default behaviour. */
	UFUNCTION(BlueprintCallable, Category = "Metaxis AI Perception Tools|Team Relationship", meta = (DisplayName = "Clear All Relationships"))
	void ClearAllRelationships();

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	UPROPERTY()
	TArray<FMetaxisTeamRelationship> TeamRelationships;

	ETeamAttitude::Type ResolveAttitude(FGenericTeamId A, FGenericTeamId B) const;
};
