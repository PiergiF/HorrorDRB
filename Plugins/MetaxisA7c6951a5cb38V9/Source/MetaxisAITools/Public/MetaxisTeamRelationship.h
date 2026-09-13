// Copyright Metaxis Games 2026, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Engine/DataTable.h"
#include "MetaxisTeamRelationship.generated.h"

USTRUCT(BlueprintType)
struct METAXISAITOOLS_API FMetaxisTeamRelationship : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Team Relationship")
	FGenericTeamId TeamA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Team Relationship")
	FGenericTeamId TeamB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Team Relationship")
	TEnumAsByte<ETeamAttitude::Type> Attitude = ETeamAttitude::Hostile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Team Relationship")
	FString DeveloperComments;
};
