// Copyright Metaxis Games 2026, All Rights Reserved

#include "MetaxisSightTargetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UMetaxisSightTargetComponent::UMetaxisSightTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bFallbackToActorLocation = true;
	PreferredMeshTag = NAME_None;
}

UAISense_Sight::EVisibilityResult UMetaxisSightTargetComponent::CanBeSeenFrom(
	const FCanBeSeenFromContext& Context,
	FVector& OutSeenLocation,
	int32& OutNumberOfLoSChecksPerformed,
	int32& OutNumberOfAsyncLosCheckRequested,
	float& OutSightStrength,
	int32* UserData,
	const FOnPendingVisibilityQueryProcessedDelegate* Delegate)
{
	OutNumberOfLoSChecksPerformed = 0;
	OutNumberOfAsyncLosCheckRequested = 0;
	OutSightStrength = 0.0f;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return UAISense_Sight::EVisibilityResult::NotVisible;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return UAISense_Sight::EVisibilityResult::NotVisible;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Context.IgnoreActor);

	USkeletalMeshComponent* Mesh = ResolveSkeletalMesh();

	if (Mesh && SightTargetSockets.Num() > 0)
	{
		for (const FName& SocketName : SightTargetSockets)
		{
			if (!Mesh->DoesSocketExist(SocketName))
			{
				continue;
			}

			const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
			FHitResult HitResult;
			++OutNumberOfLoSChecksPerformed;

			const bool bHit = World->LineTraceSingleByChannel(
				HitResult,
				Context.ObserverLocation,
				SocketLocation,
				ECC_Visibility,
				QueryParams);

			if (!bHit || HitResult.GetActor() == OwnerActor)
			{
				OutSeenLocation = SocketLocation;
				OutSightStrength = 1.0f;
				return UAISense_Sight::EVisibilityResult::Visible;
			}
		}
	}

	if (bFallbackToActorLocation)
	{
		const FVector ActorLocation = OwnerActor->GetActorLocation();
		FHitResult HitResult;
		++OutNumberOfLoSChecksPerformed;

		const bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			Context.ObserverLocation,
			ActorLocation,
			ECC_Visibility,
			QueryParams);

		if (!bHit || HitResult.GetActor() == OwnerActor)
		{
			OutSeenLocation = ActorLocation;
			OutSightStrength = 1.0f;
			return UAISense_Sight::EVisibilityResult::Visible;
		}
	}

	return UAISense_Sight::EVisibilityResult::NotVisible;
}

USkeletalMeshComponent* UMetaxisSightTargetComponent::ResolveSkeletalMesh() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	OwnerActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	// 1) Prefer a mesh that matches the tag and contains at least one target socket.
	if (!PreferredMeshTag.IsNone())
	{
		for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
		{
			if (MeshComp && MeshComp->ComponentHasTag(PreferredMeshTag))
			{
				return MeshComp;
			}
		}
	}

	// 2) Character mesh fallback.
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
	{
		USkeletalMeshComponent* CharMesh = OwnerCharacter->GetMesh();
		if (CharMesh)
		{
			return CharMesh;
		}
	}

	// 3) First available skeletal mesh.
	for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
	{
		if (MeshComp)
		{
			return MeshComp;
		}
	}

	return nullptr;
}
