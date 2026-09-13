// Copyright Metaxis Games 2026, All Rights Reserved

#include "MetaxisEyeSourceComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMetaxisEyeSourceComponent::UMetaxisEyeSourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bEnableOverrideEyesViewpoint = true;
	EyeSocketName = TEXT("Eyes");
	PreferredEyeMeshTag = NAME_None;
	bFlattenSightForwardAxis = false;
}

bool UMetaxisEyeSourceComponent::GetCustomEyesViewPoint(const APawn* TargetPawn, FVector& OutLocation, FRotator& OutRotation) const
{

	if (!TargetPawn || !bEnableOverrideEyesViewpoint)
	{
		return false;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	TargetPawn->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	TSet<const USkeletalMeshComponent*> TriedMeshes;

	auto TryUseMeshForSocket = [&](USkeletalMeshComponent* CandidateMesh) -> bool
	{
		if (!CandidateMesh || TriedMeshes.Contains(CandidateMesh))
		{
			return false;
		}

		TriedMeshes.Add(CandidateMesh);
		if (!CandidateMesh->DoesSocketExist(EyeSocketName))
		{
			return false;
		}

		const FTransform SocketTransform = CandidateMesh->GetSocketTransform(EyeSocketName);
		OutLocation = SocketTransform.GetLocation();

		if (bFlattenSightForwardAxis)
		{
			FVector SocketForwardVector = SocketTransform.GetRotation().GetForwardVector();
			FVector FlatForwardVector = SocketForwardVector;
			FlatForwardVector.Z = 0.0f;

			if (!FlatForwardVector.IsNearlyZero())
			{
				FlatForwardVector.Normalize();
			}
			else
			{
				FlatForwardVector = TargetPawn->GetActorForwardVector();
				FlatForwardVector.Z = 0.0f;
				if (!FlatForwardVector.IsNearlyZero())
				{
					FlatForwardVector.Normalize();
				}
				else
				{
					FlatForwardVector = FVector::ForwardVector;
				}
			}
			OutRotation = UKismetMathLibrary::MakeRotFromX(FlatForwardVector);
		}
		else
		{
			OutRotation = SocketTransform.GetRotation().Rotator();
		}

		return true;
	};

	// 1) Explicit priority: tagged meshes that contain the target socket.
	if (!PreferredEyeMeshTag.IsNone())
	{
		for (USkeletalMeshComponent* MeshComponent : SkeletalMeshes)
		{
			if (MeshComponent && MeshComponent->ComponentHasTag(PreferredEyeMeshTag) && TryUseMeshForSocket(MeshComponent))
			{
				return true;
			}
		}
	}

	// 2) Character-friendly fallback: evaluate the primary Character mesh next.
	const ACharacter* OwnerCharacter = Cast<ACharacter>(TargetPawn);
	if (OwnerCharacter && TryUseMeshForSocket(OwnerCharacter->GetMesh()))
	{
		return true;
	}

	// 3) Final fallback: check all remaining skeletal meshes in component order.
	for (USkeletalMeshComponent* MeshComponent : SkeletalMeshes)
	{
		if (TryUseMeshForSocket(MeshComponent))
		{
			return true;
		}
	}

	return false;
}

void UMetaxisEyeSourceComponent::SetFlattenSightForwardAxis(AAIController* OwningController, bool bNewEnabled)
{
	bFlattenSightForwardAxis = bNewEnabled;
	UE_LOG(LogTemp, Log, TEXT("UMetaxisEyeSourceComponent '%s': Set Compensate Sight Forward Axis to %s."),
		   *GetName(), bNewEnabled ? TEXT("Enabled") : TEXT("Disabled"));

	if (OwningController)
	{
		UAIPerceptionComponent* PerceptionComp = OwningController->GetPerceptionComponent();
		if (PerceptionComp)
		{
			PerceptionComp->RequestStimuliListenerUpdate();
		}
	}
}