#include "EosGameInstanceDRB.h"
#include "HorrorDRB5_8/PlayerControllerDRB.h"

DEFINE_LOG_CATEGORY(LogGameInstance);

/// <summary>
/// Initialize Game Instance object
/// </summary>
void UEosGameInstanceDRB::Init()
{
	UE_LOG(LogGameInstance, Log, TEXT("EosGameInstanceDRB initialized."));
	Super::Init();
}

/// <summary>
/// Shutdown Game Instance object
/// </summary>
void UEosGameInstanceDRB::Shutdown()
{
	UE_LOG(LogGameInstance, Log, TEXT("EosGameInstanceDRB shutdown."));
	Super::Shutdown();
}

/// <summary>
/// Initialize Game Instance object
/// </summary>
/// <param name="ObjectInitializer"></param>
UEosGameInstanceDRB::UEosGameInstanceDRB(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

/// <summary>
/// Retrieve a reference to the primary player controller
/// </summary>
/// <returns>AOnlineSamplePlayerController pointer</returns>
APlayerControllerDRB* UEosGameInstanceDRB::GetPrimaryPlayerController() const
{
	return Cast<APlayerControllerDRB>(Super::GetPrimaryPlayerController(false));
}