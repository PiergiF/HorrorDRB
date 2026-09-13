// DRB
#include "GameModeBaseDRB.h"

#include "HorrorDRB5_8/PlayerControllerDRB.h"
#include "HorrorDRB5_8/MainCharacterDRB.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerControllerDRB.h"

AGameModeBaseDRB::AGameModeBaseDRB()
{
	//GameStateClass = AGameStateDRB::StaticClass();

	UE_LOG(LogTemp, Warning, TEXT("***DRB*** game mode starting"));

	//PlayerStateClass = APlayerStateDRB::StaticClass();
	PlayerControllerClass = APlayerControllerDRB::StaticClass();

	// Set default pawn class to our character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/DRB/BP_MainCharacterDRB"));
	if (PlayerPawnClassFinder.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("***DRB*** bp character class not found"));
	}
}

void AGameModeBaseDRB::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("***DRB*** Game Mode has started"));
}