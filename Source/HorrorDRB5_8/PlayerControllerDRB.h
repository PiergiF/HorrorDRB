#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerDRB.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class HORRORDRB5_8_API APlayerControllerDRB : public APlayerController
{
	GENERATED_BODY()

	/*************************DRB CODE****************************************/

	//--------------------PROPERTY----------------------//

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	//--------------------FUNCTIONS----------------------//

public:

	/** Constructor */
	APlayerControllerDRB();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Called once play ends */
	virtual void EndPlay(EEndPlayReason::Type EndReason);
};
