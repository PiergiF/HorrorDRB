// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterDRB.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"

class BP_GameplayTagsManager;

// Sets default values
AMainCharacterDRB::AMainCharacterDRB()
{

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps. This is the configuration for the mesh that is see by other players
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

}

// Called when the game starts or when spawned
void AMainCharacterDRB::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMainCharacterDRB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacterDRB::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMainCharacterDRB::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMainCharacterDRB::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacterDRB::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainCharacterDRB::LookInput);
		//EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AprovaPrimPersCodeCharacter::LookInput);

		//Shoot
		//EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AprovaPrimPersCodeCharacter::StartFire);

		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AMainCharacterDRB::Interact);
	}
	else
	{
		//UE_LOG(LogMainCharacterDRB, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		UE_LOG(LogTemp, Warning, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}


void AMainCharacterDRB::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}


void AMainCharacterDRB::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMainCharacterDRB::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AMainCharacterDRB::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


void AMainCharacterDRB::Interact()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("In interact function"));
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + FirstPersonCameraComponent->GetForwardVector() * 500.0f; //deve essere ditante massimo 500 cm. in avarti

	//const FName TraceTag("MyTraceTag");

	//GetWorld()->DebugDrawTraceTag = TraceTag;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	//Params.TraceTag = TraceTag;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) //or ECollisionChannel::
	{
		//UPrimitiveComponent* ComponentHit = HitResult.GetComponent();
		//HitResult.GetHitObjectHandle();
		//AActor* ActorHit = HitResult.GetActor();

		//con component

		if (UPrimitiveComponent* ComponentHit = HitResult.GetComponent())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("FIRST IF TRUE"));
			//ActorHit->TagSubobjects
			static const FName InteractableTag = TEXT("Interactable");
			//bool b = ActorHit->ActorHasTag(InteractableTag);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("b is: '%b'"), b);
			//TArray<FName> a = ActorHit->Tags;
			if (ComponentHit->ComponentHasTag(InteractableTag))
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("SECOND IF TRUE"));
				//UE_LOG(LogTemp, Warning, TEXT("Hit Interactable Actor: '%s'"), *ActorHit->GetName());
				UE_LOG(LogTemp, Warning, TEXT("Hit Interactable Component: '%s'"), *ComponentHit->GetName());
			}
		}


		//con actor(gameplaytags)
		/*
		if (AActor* ActorHit = HitResult.GetActor())
		{
			BP_GameplayTagsManager* a = ActorHit->GetComponentByClass(UActorComponent* palle);
		}
		*/
	}

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f, 0, 5.0f);
}

void AMainCharacterDRB::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AMainCharacterDRB::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}