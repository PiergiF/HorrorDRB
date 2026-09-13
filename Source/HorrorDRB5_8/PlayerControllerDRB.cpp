#include "PlayerControllerDRB.h"
#include "EnhancedInputSubsystems.h"
//#include "Survival_Horror/EOSGameInstanceDRB.h"
//#include "Survival_Horror/EOSGameInstanceSubsystemDRB.h"

APlayerControllerDRB::APlayerControllerDRB()
{
	// set the player camera manager class
	//PlayerCameraManagerClass = ACameraManagerDRB::StaticClass();
}

void APlayerControllerDRB::BeginPlay()
{
	Super::BeginPlay();

	/*
	////////////////////////////////////////////////
	/// ONLINE SERVICES

	// Register this player with the Online Services
	UEOSGameInstanceDRB* GameInstance = Cast<UEOSGameInstanceDRB>(GetWorld()->GetGameInstance());
	UEOSGameInstanceSubsystemDRB* OnlineSubsystem = GameInstance->GetSubsystem<UEOSGameInstanceSubsystemDRB>();
	ULocalPlayer* LocalPlayer = Super::GetLocalPlayer();
	if (LocalPlayer)
	{
		FPlatformUserId LocalPlayerPlatformUserId = LocalPlayer->GetPlatformUserId();
		if (OnlineSubsystem) // null-check subsystem before access
		{
			UE_LOG(LogEOSGameInstanceSubsystemDRB, Log, TEXT("Registering PlatformUserId: %d"), LocalPlayerPlatformUserId.GetInternalId());
			OnlineSubsystem->RegisterLocalOnlineUser(LocalPlayerPlatformUserId);

			// Read the Title File and display contents on-screen
			FString TitleFileContent = OnlineSubsystem->ReadTitleFile(FString("StatusFile"), LocalPlayerPlatformUserId);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Black, TitleFileContent);
			}
		}
	}
	 */
	 ///////////////////////////////////////////////
	 /// NEXT SECTION...
}

void APlayerControllerDRB::EndPlay(EEndPlayReason::Type EndReason)
{
	Super::EndPlay(EndReason);
}

void APlayerControllerDRB::SetupInputComponent()
{

	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

		}
	}
}
