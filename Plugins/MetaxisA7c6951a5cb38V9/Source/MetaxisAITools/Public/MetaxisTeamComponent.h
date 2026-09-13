// Copyright Metaxis Games 2026, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaxisTeamComponent.generated.h"

/**
 * Assigns a team ID to any actor without requiring IGenericTeamAgentInterface.
 * Attach this component to pawns, characters, or any actor that should
 * participate in team-based perception filtering.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXISAITOOLS_API UMetaxisTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMetaxisTeamComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Team Component")
	uint8 TeamID;

	UFUNCTION(BlueprintCallable, Category = "Metaxis AI Perception Tools|Team Component", meta = (DisplayName = "Switch Team"))
	void SwitchTeam(uint8 NewTeamID);
};
