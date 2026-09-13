#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EosGameInstanceDRB.generated.h"

// Forward Declare classes
class APlayerControllerDRB;
class UObject;

DECLARE_LOG_CATEGORY_EXTERN(LogGameInstance, Log, All);

/**
 *
 */
UCLASS()
class HORRORDRB5_8_API UEosGameInstanceDRB : public UGameInstance
{
	GENERATED_BODY()

protected:

	/** Called to initialize game instance on game startup */
	virtual void Init() override;
	/** Called to shutdown game instance on game exit */
	virtual void Shutdown() override;

public:
	/** Called to initialize game instance object */
	UEosGameInstanceDRB(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/** Called to retrieve the primary player controller */
	APlayerControllerDRB* GetPrimaryPlayerController() const;
};
