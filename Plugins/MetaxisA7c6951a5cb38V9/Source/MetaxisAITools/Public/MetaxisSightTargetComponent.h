// Copyright Metaxis Games 2026, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AISightTargetInterface.h"
#include "MetaxisSightTargetComponent.generated.h"

/**
 * Defines which body parts on an actor trigger AI sight perception.
 * Attach to the actor being perceived (e.g. the player character).
 *
 * The engine's UAISense_Sight automatically discovers this component via
 * FindComponentByInterface<IAISightTargetInterface>() and delegates
 * line-of-sight checks to CanBeSeenFrom.
 *
 * Each entry in SightTargetSockets is a bone/socket name on the owning
 * actor's skeletal mesh. The AI will only "see" the actor when a clear
 * line of sight exists to one of those sockets.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class METAXISAITOOLS_API UMetaxisSightTargetComponent : public UActorComponent, public IAISightTargetInterface
{
	GENERATED_BODY()

public:
	UMetaxisSightTargetComponent();

	/**
	 * Bone or socket names to use as sight target points.
	 * The AI traces from its eye location to each of these sockets in order;
	 * if any trace is unobstructed the actor is considered visible at that point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Sight Target Component")
	TArray<FName> SightTargetSockets;

	/**
	 * Optional component tag used to identify which skeletal mesh to resolve
	 * sockets from. Meshes with this tag are checked first. Follows the same
	 * pattern as UMetaxisEyeSourceComponent::PreferredEyeMeshTag.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Sight Target Component")
	FName PreferredMeshTag;

	/**
	 * If true and no sockets can be resolved (mesh missing, socket names
	 * invalid, etc.), the component falls back to the actor's origin for
	 * line-of-sight checks. If false, the actor is treated as not visible.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metaxis AI Perception Tools|Sight Target Component")
	bool bFallbackToActorLocation;

	// IAISightTargetInterface
	virtual UAISense_Sight::EVisibilityResult CanBeSeenFrom(
		const FCanBeSeenFromContext& Context,
		FVector& OutSeenLocation,
		int32& OutNumberOfLoSChecksPerformed,
		int32& OutNumberOfAsyncLosCheckRequested,
		float& OutSightStrength,
		int32* UserData = nullptr,
		const FOnPendingVisibilityQueryProcessedDelegate* Delegate = nullptr) override;

private:

	USkeletalMeshComponent* ResolveSkeletalMesh() const;
};
